#include <stdio.h>
#include "AX_servo.h"

AX_conf_t conf;

void app_main(void)
{
    conf.uart = UART_NUM_1;
    conf.tx_pin = 17;
    conf.rx_pin = 18;
    conf.rts_pin = 7;      // TX_EN
    conf.baudrate = 57600;

    AX_servo_init(conf);
    while(1){
        AX_servo_set_max_torque(conf, 1, 512);
        uint16_t response1 = AX_servo_get_pos(conf, 1);
        AX_servo_set_pos_w_spd(conf, 1, 702, 1023);

        //printf("%d", response1);
        vTaskDelay(3000/portTICK_PERIOD_MS);

        AX_servo_set_max_torque(conf, 1, 512);
        uint16_t response2 = AX_servo_get_pos(conf, 1);
        AX_servo_set_pos_w_spd(conf, 1, 500, 1023);

        vTaskDelay(3000/portTICK_PERIOD_MS);
        // printf("%d", response2);
    }
    
}