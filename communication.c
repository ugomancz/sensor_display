/*
 * communication.c
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */
#include "communication.h"
#include "application.h"
#include "crc.h"
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void set_comm_direction(direction dir) {
    GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_4, (dir == TRANSMIT) ? 0xFF : 0x00);
}

void send_message() {
    set_comm_direction(TRANSMIT);
    switch (current_context) {
    case FIND:
        _get_dev_id();
        break;
    case MENU:
        _get_dev_temp();
        break;
    case DOSE:
        _get_dev_dose();
        break;
    case DOSE_RATE:
        _get_dev_dose_rate();
        break;
    }
}

void _get_dev_id() {
    const uint8_t message[] = { device_address, 0x04, 0x00, 0x00, 0x00, 0x24 };
    const uint16_t crc = get_crc(message, 6);
    for (short i = 0; i < 6; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
    UARTCharPut(UART6_BASE, (uint8_t) crc);
    UARTCharPut(UART6_BASE, (uint8_t) (crc >> 8));
}

void _get_dev_temp() {
    const uint8_t message[] = { device_address, 0x04, 0x01, 0x14, 0x00, 0x0A };
    const uint16_t crc = get_crc(message, 6);
    for (short i = 0; i < 6; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
    UARTCharPut(UART6_BASE, (uint8_t) crc);
    UARTCharPut(UART6_BASE, (uint8_t) (crc >> 8));
}

void _get_dev_dose() {
    const uint8_t message[] = { device_address, 0x04, 0x01, 0x0A, 0x00, 0x0A };
    const uint16_t crc = get_crc(message, 6);
    for (short i = 0; i < 6; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
    UARTCharPut(UART6_BASE, (uint8_t) crc);
    UARTCharPut(UART6_BASE, (uint8_t) (crc >> 8));
}

void _get_dev_dose_rate() {
    const uint8_t message[] = { device_address, 0x04, 0x01, 0x00, 0x00, 0x0A };
    const uint16_t crc = get_crc(message, 6);
    for (short i = 0; i < 6; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
    UARTCharPut(UART6_BASE, (uint8_t) crc);
    UARTCharPut(UART6_BASE, (uint8_t) (crc >> 8));
}

void parse_received() {
    /* Check correct device address, function code and message length */
    if (buffer[0] != device_address || buffer[1] != 0x04 || buffer[2] != buffer_position - 5) {
        reset_buffer();
        return;
    }
    /* Check CRC */
    uint16_t message_crc = ((int16_t) buffer[buffer_position - 1] << 8) | buffer[buffer_position - 2];
    if (message_crc != get_crc(buffer, buffer_position - 2)) {
        reset_buffer();
        return;
    }
    switch (current_context) {
    case FIND:
        memcpy(&device_id, buffer + 3, buffer[2]);
        switch_string_endianity(device_id.pr_id);
        switch_string_endianity(device_id.pr_name);
        reset_buffer();
        break;
    case MENU:
    case DOSE:
    case DOSE_RATE:
        memcpy(&ch_value, buffer + 3, buffer[2]);
        switch_float_endianity(&(ch_value.val));
        reset_buffer();
        break;
    }

}

void reset_buffer() {
    buffer_position = 0;
    memset(buffer, 0, 256);
}
