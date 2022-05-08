/*
 * communication.c
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */
#include "communication.h"
#include "application.h"
#include "modbus.h"
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <stdlib.h>
#include <string.h>

volatile comm_context current_comm_context = DEVICE_LOOKUP;
volatile comm_state current_comm_state = SEND_MESSAGE;

volatile uint8_t device_address = 0x01;
volatile uint8_t device_lookup_address = 0x01;
volatile uint8_t comm_error_counter = 0;

uint8_t *tx_buffer;
uint8_t *rx_buffer;
volatile uint8_t tx_buffer_pos = 0;
volatile uint8_t rx_buffer_pos = 0;

dev_id device_id = { 0 };
dev_id device_lookup_id = { 0 };
ch_val ch_values[3] = { 0 };
ch_par ch_pars[3] = { 0 };
uint16_t last_par_cnts[3] = { 0 };

void set_comm_direction(direction dir) {
    GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_4, (dir == TRANSMIT) ? 0xFF : 0x00);
}

int init_comm_buffers() {
    if (((tx_buffer = calloc(TX_BUFFER_SIZE, sizeof(uint8_t))) == NULL)
            || ((rx_buffer = calloc(RX_BUFFER_SIZE, sizeof(uint8_t))) == NULL)) {
        return -1;
    }
    return 0;
}

void reset_tx_buffer() {
    memset(tx_buffer, 0, tx_buffer_pos);
    tx_buffer_pos = 0;
}

void reset_rx_buffer() {
    memset(rx_buffer, 0, rx_buffer_pos);
    rx_buffer_pos = 0;
}

void uart_send() {
    set_comm_direction(TRANSMIT);
    for (int i = 0; i < tx_buffer_pos; ++i) {
        UARTCharPut(UART6_BASE, tx_buffer[i]);
    }
}

void send_request() {
    switch (current_comm_context) {
    case DEVICE_LOOKUP:
        gen_mb_read_input_regs(device_lookup_address, ID_REG_START_ADDR, ID_REGS_COUNT, tx_buffer, &tx_buffer_pos);
        break;
    case FETCH_CH_PARS:
        gen_mb_read_input_regs(device_address, DOSE_RATE_PAR_REG_START_ADDR, CH_PAR_REGS_COUNT * 3, tx_buffer, &tx_buffer_pos);
        break;
    case FETCH_CH_VALUES:
        gen_mb_read_input_regs(device_address, DOSE_RATE_REG_START_ADDR, CH_VAL_REGS_COUNT * 3, tx_buffer, &tx_buffer_pos);
        break;
    }
    uart_send();
    reset_tx_buffer();
}

int process_requested_data() {
    int retval;
    switch (current_comm_context) {
    case DEVICE_LOOKUP:
        retval = decode_mb_read_input_regs(rx_buffer, rx_buffer_pos, &device_lookup_id);
        break;
    case FETCH_CH_VALUES:
        retval = decode_mb_read_input_regs(rx_buffer, rx_buffer_pos, &ch_values);
        break;
    case FETCH_CH_PARS:
        retval = decode_mb_read_input_regs(rx_buffer, rx_buffer_pos, &ch_pars);
        if (retval == SUCCESS) {
            last_par_cnts[DOSE_RATE_CH] = ch_values[DOSE_RATE_CH].par_cnt;
            last_par_cnts[DOSE_CH] = ch_values[DOSE_CH].par_cnt;
            last_par_cnts[TEMP_CH] = ch_values[TEMP_CH].par_cnt;
        }
        break;
    }
    if (retval == SUCCESS) {
        reset_rx_buffer();
    }
    return retval;
}
