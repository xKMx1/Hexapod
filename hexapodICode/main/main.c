#include <stdio.h>
#include <string.h>
#include "AX_servo.h"     // pulls in FreeRTOS, driver/gpio.h, driver/uart.h

// ============================================================================
//  HEXAPOD - move one servo, transmit only.
//
//  Target is ID 14 at 57600 baud. Every other baud rate is tried afterwards
//  in case this servo was reflashed, but 57600 always goes first so you see
//  the expected case immediately.
//
//  Nothing here waits for a status packet. RX on this board is not trusted
//  (RXEN1/RXEN2 come only from the 74LVC14A), so the servo itself is the
//  only instrument: watch the horn, not the terminal.
// ============================================================================

#define UART_PORT      UART_NUM_1

#define TARGET_ID      14
#define DEFAULT_BAUD   57600

// AX-12A position units: 1023 counts over 300 deg -> 3.41 counts per degree.
#define CENTER_POS     512      // mechanical middle
#define MOVE_DEG       20       // total sweep, degrees. Raise if you can't see it.
#define MOVE_SPEED     200      // 0..1023, 1023 ~= 114 RPM. Gentle.

#define DEG2POS(d)     ((int)((d) * 1023.0 / 300.0 + 0.5))
#define POS_LO         (CENTER_POS - DEG2POS(MOVE_DEG) / 2)
#define POS_HI         (CENTER_POS + DEG2POS(MOVE_DEG) / 2)

#define AX_REG_TORQUE_LIM 34      // RAM, 2 bytes
#define AX_REG_TORQUE_EN  24      // RAM, 1 byte
#define AX_REG_LED        25      // RAM, 1 byte
#define AX_REG_GOAL_POS   30      // RAM, 4 bytes: goal_L goal_H speed_L speed_H

typedef struct { const char *name; int txen, tx, rx; } bus_t;

static const bus_t buses[] = {
    { "Bus 1  (J3/J4/J5, rail VBUS_2)", 7, 17, 18 },
    { "Bus 2  (J6/J7/J8, rail VBUS_1)", 8,  5,  4 },
};
#define N_BUSES (sizeof(buses)/sizeof(buses[0]))

// DEFAULT_BAUD first on purpose. The rest are fallbacks.
static const uint32_t bauds[] = { DEFAULT_BAUD, 1000000, 115200, 500000, 19200, 9600 };
#define N_BAUDS (sizeof(bauds)/sizeof(bauds[0]))

static void cfg_out(int pin)
{
    gpio_config_t c = {
        .pin_bit_mask = (1ULL << pin), .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE, .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&c);
}

static void ax_write_reg(const bus_t *b, uint8_t id, uint8_t addr,
                         const uint8_t *data, int n)
{
    uint8_t pkt[16];
    uint8_t len = (uint8_t)(n + 3);       // params(addr + n) + 2
    int k = 0;
    pkt[k++] = AX_START; pkt[k++] = AX_START;
    pkt[k++] = id; pkt[k++] = len;
    pkt[k++] = 0x03;                      // WRITE_DATA
    pkt[k++] = addr;
    memcpy(&pkt[k], data, n); k += n;

    uint32_t sum = 0;
    for (int i = 2; i < k; i++) sum += pkt[i];
    pkt[k++] = (uint8_t)(~sum & 0xFF);

    gpio_set_level(b->txen, 0);           // /OE low -> transmit gate ON
    uart_write_bytes(UART_PORT, (const char *)pkt, k);
    uart_wait_tx_done(UART_PORT, pdMS_TO_TICKS(100));
    gpio_set_level(b->txen, 1);           // release the bus
}

static void open_uart(const bus_t *b, uint32_t baud)
{
    uart_config_t cfg = {
        .baud_rate = (int)baud, .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_DEFAULT,
    };
    if (uart_is_driver_installed(UART_PORT)) uart_driver_delete(UART_PORT);
    uart_driver_install(UART_PORT, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT, b->tx, b->rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void goto_pos(const bus_t *b, uint8_t id, int pos, int speed)
{
    uint8_t a[4] = {
        (uint8_t)(pos & 0xFF),   (uint8_t)(pos >> 8),
        (uint8_t)(speed & 0xFF), (uint8_t)(speed >> 8),
    };
    ax_write_reg(b, id, AX_REG_GOAL_POS, a, 4);
}

// One small wag: enable torque, nudge one way, nudge back. LED marks each leg
// so a servo that is powered and listening but mechanically stuck still shows.
static void small_wag(const bus_t *b, uint8_t id)
{
    uint8_t one = 1, zero = 0;
    uint8_t lim[2] = { 0xFF, 0x03 };                   // torque limit 1023

    ax_write_reg(b, id, AX_REG_TORQUE_LIM, lim, 2);    // also clears alarm shutdown
    vTaskDelay(pdMS_TO_TICKS(10));
    ax_write_reg(b, id, AX_REG_TORQUE_EN, &one, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    ax_write_reg(b, id, AX_REG_LED, &one, 1);
    goto_pos(b, id, POS_LO, MOVE_SPEED);
    vTaskDelay(pdMS_TO_TICKS(900));

    ax_write_reg(b, id, AX_REG_LED, &zero, 1);
    goto_pos(b, id, POS_HI, MOVE_SPEED);
    vTaskDelay(pdMS_TO_TICKS(900));

    goto_pos(b, id, CENTER_POS, MOVE_SPEED);           // leave it centred
    vTaskDelay(pdMS_TO_TICKS(600));
}

void app_main(void)
{
    // FIRST THING: release both buses. R3/R4 pull TXEN low, which ENABLES the
    // transmit buffers while GPIO7/GPIO8 are still high-Z during boot, so the
    // board drives garbage onto the servo bus for the first few hundred ms.
    for (int i = 0; i < (int)N_BUSES; i++) {
        cfg_out(buses[i].txen);
        gpio_set_level(buses[i].txen, 1);
    }

    vTaskDelay(pdMS_TO_TICKS(500));
    printf("\n\n============ MOVE SERVO ID %d ============\n", TARGET_ID);
    printf("  default baud : %d\n", DEFAULT_BAUD);
    printf("  sweep        : %d deg  (%d <-> %d, centre %d, speed %d)\n",
           MOVE_DEG, POS_LO, POS_HI, CENTER_POS, MOVE_SPEED);
    printf("  transmit only - nothing waits for a reply. Watch the servo.\n");
    printf("  Note the baud printed when it twitches or its LED blinks.\n");
    printf("  Repeats forever; Ctrl+] to quit the monitor.\n");

    for (int pass = 1; ; pass++) {
        printf("\n=================== PASS %d ===================\n", pass);

        for (int k = 0; k < (int)N_BAUDS; k++) {
            for (int i = 0; i < (int)N_BUSES; i++) {
                const bus_t *b = &buses[i];

                cfg_out(b->txen);
                gpio_set_level(b->txen, 1);
                // park the other bus so the two cannot interfere
                for (int j = 0; j < (int)N_BUSES; j++)
                    if (j != i) gpio_set_level(buses[j].txen, 1);

                open_uart(b, bauds[k]);
                printf("  baud %7lu%-10s %-32s ID %-3d ... ",
                       (unsigned long)bauds[k],
                       (bauds[k] == DEFAULT_BAUD) ? " (default)" : "",
                       b->name, TARGET_ID);
                fflush(stdout);
                small_wag(b, TARGET_ID);
                printf("sent\n");
            }
        }
    }
}
