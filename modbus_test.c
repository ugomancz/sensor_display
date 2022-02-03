#include "modbus_test.h"
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

frame create_frame(uint8_t slave_address, uint8_t function_code, uint8_t data[], uint16_t crc) {
    frame f;
    f.slave_address = slave_address;
    f.function_code = function_code;
    f.data_length = (uint8_t) (sizeof(data) / sizeof(uint8_t));
    f.crc = crc;
    f.data = calloc(f.data_length, sizeof(uint8_t));
    for (int i = 0; i < f.data_length; i++) {
        f.data[i] = data[i];
    }
    return f;
}

int uart_send_frame(uint32_t uart_base, frame message) {
    UARTCharPut(uart_base, message.slave_address);
    UARTCharPut(uart_base, message.function_code);
    for (int i = 0; i < message.data_length; i++) {
        UARTCharPut(uart_base, message.data[i]);
    }
    UARTCharPut(uart_base, (uint8_t) (message.crc >> 8));
    UARTCharPut(uart_base, (uint8_t) message.crc);
    return 0;
}

void set_direction(direction dir) {
    GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_4, (dir == TRANSMIT) ? 0xFF : 0x00);
    de_nre = dir;
}

frame parse_incoming_message(unsigned char buffer[]) {
    size_t buffer_size = strlen((const char *) buffer);
    printf("Buffer size: %d\n", (int) buffer_size);
    frame f = { { 0 } };

    // TODO parse buffer to a frame
    return f;
}
