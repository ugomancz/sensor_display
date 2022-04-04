#include <communication.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc.h"
#include "application.h"


void set_direction(direction dir) {
    GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_4, (dir == TRANSMIT) ? 0xFF : 0x00);
}

void send_message() {
    set_direction(TRANSMIT);
    switch (current_context) {
    case FIND:
        _get_dev_id();
        break;
    }
}

void _get_dev_id() {
    const uint8_t message[] = {device_address, 0x04, 0x00, 0x00, 0x00, 0x24, 0xF0, 0x11}; // add calculated crc
    for (short i = 0; i < 8; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
}


void parse_received() {
    /* Check correct device address, function code and message length */
    if (buffer[0] != device_address || buffer[1] != 0x04 || buffer[2] != buffer_position - 5) {
        reset_buffer();
        return;
    }
    /* Check CRC */
    uint16_t message_crc = buffer[buffer_position - 2];
    message_crc <<= 8;
    message_crc |= buffer[buffer_position - 1];
    if (message_crc != to_modbus_compatible(get_crc(buffer, buffer_position - 2))) {
        reset_buffer();
        return;
    }
    switch (current_context) {
        case FIND:
            memcpy(&device_id, buffer + 3, buffer[2]);
            break;
        }
}

void reset_buffer() {
    buffer_position = 0;
    memset(buffer, 0, 256);
}
