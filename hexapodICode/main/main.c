#include <stdio.h>
#include "AX_servo.h"

AX_conf_t conf;

void app_main(void)
{
    conf.uart = UART_NUM_2;
    conf.tx_pin = 17;
    conf.rx_pin = 16;
    conf.rts_pin = 34;  // TX_EN
    conf.baudrate = 57600;

    u_int8_t res;
    AX_servo_init(conf);
    AX_servo_ping(conf, 1);
}