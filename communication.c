/*
 * communication.c
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */
#include "communication.h"
#include "application.h"
#include "crc.h"
#include "history.h"
#include "modbus.h"
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
    uint8_t message[20] = { 0 };
    uint8_t message_len = 0;
    gen_mb_read_input_regs(device_address, ID_REG_START_ADDR, ID_REGS_COUNT, message, &message_len);
    for (short i = 0; i < message_len; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
}

void _get_dev_temp() {
    uint8_t message[20] = { 0 };
    uint8_t message_len = 0;
    gen_mb_read_input_regs(device_address, TEMP_REG_START_ADDR, CH_VAL_REGS_COUNT, message, &message_len);
    for (short i = 0; i < message_len; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
}

void _get_dev_dose() {
    uint8_t message[20] = { 0 };
    uint8_t message_len = 0;
    gen_mb_read_input_regs(device_address, DOSE_REG_START_ADDR, CH_VAL_REGS_COUNT, message, &message_len);
    for (short i = 0; i < message_len; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
}

void _get_dev_dose_rate() {
    uint8_t message[20] = { 0 };
    uint8_t message_len = 0;
    gen_mb_read_input_regs(device_address, DOSE_RATE_REG_START_ADDR, CH_VAL_REGS_COUNT, message, &message_len);
    for (short i = 0; i < message_len; ++i) {
        UARTCharPut(UART6_BASE, message[i]);
    }
}

void parse_received() {
    switch (current_context) {
    case FIND:
        decode_mb_read_input_regs(buffer, buffer_position, &device_id);
        reset_buffer();
        break;
    case MENU:
    case DOSE:
    case DOSE_RATE:
        decode_mb_read_input_regs(buffer, buffer_position, &ch_value);
        add_value_to_history(ch_value.val);
        reset_buffer();
        break;
    }

}

void reset_buffer() {
    buffer_position = 0;
    memset(buffer, 0, RX_BUFFER_SIZE);
}
