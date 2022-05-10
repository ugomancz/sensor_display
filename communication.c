/*
 * communication.c
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */
#include "communication.h"
#include "modbus.h"
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <stdlib.h>
#include <string.h>

volatile comm_context current_comm_context = DEVICE_LOOKUP;
volatile comm_state current_comm_state = SEND_MESSAGE;

volatile uint8_t comm_error_counter = 0;

uint8_t *tx_buffer;
uint8_t *rx_buffer;
volatile uint8_t tx_buffer_pos = 0;
volatile uint8_t rx_buffer_pos = 0;

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

/* Resets TX buffer */
static void reset_tx_buffer() {
    memset(tx_buffer, 0, tx_buffer_pos);
    tx_buffer_pos = 0;
}

/* Resets RX buffer */
static void reset_rx_buffer() {
    memset(rx_buffer, 0, rx_buffer_pos);
    rx_buffer_pos = 0;
}

/* Sends the data in TX buffer to the UART */
static void uart_send() {
    set_comm_direction(TRANSMIT);
    for (int i = 0; i < tx_buffer_pos; ++i) {
        UARTCharPut(UART6_BASE, tx_buffer[i]);
    }
}

void send_request() {
    switch (current_comm_context) {
    case DEVICE_LOOKUP:
        gen_mb_read_input_regs(lookup_sensor.addr, ID_REG_START_ADDR, ID_REGS_COUNT, tx_buffer, &tx_buffer_pos);
        break;
    case FETCH_CH_PARS:
        gen_mb_read_input_regs(current_sensor.addr, DOSE_RATE_PAR_REG_START_ADDR, CH_PAR_REGS_COUNT * 3, tx_buffer,
                &tx_buffer_pos);
        break;
    case FETCH_CH_VALUES:
        gen_mb_read_input_regs(current_sensor.addr, DOSE_RATE_REG_START_ADDR, CH_VAL_REGS_COUNT * 3, tx_buffer,
                &tx_buffer_pos);
        break;
    }
    uart_send();
    reset_tx_buffer();
}

int process_requested_data(channels_data *ch_data, par_cnts *old_par_cnts) {
    int retval;
    switch (current_comm_context) {
    case DEVICE_LOOKUP:
        retval = decode_mb_read_input_regs(rx_buffer, rx_buffer_pos, &lookup_sensor.id);
        break;
    case FETCH_CH_VALUES:
        retval = decode_mb_read_input_regs(rx_buffer, rx_buffer_pos, &ch_data->dose_rate_val);
        break;
    case FETCH_CH_PARS:
        retval = decode_mb_read_input_regs(rx_buffer, rx_buffer_pos, &ch_data->dose_rate_par);
        if (retval == SUCCESS) {
            old_par_cnts->dose_rate = ch_data->dose_rate_par.par_cnt;
            old_par_cnts->dose = ch_data->dose_par.par_cnt;
            old_par_cnts->temp = ch_data->temp_par.par_cnt;

        }
        break;
    }
    if (retval == SUCCESS) {
        reset_rx_buffer();
        if (comm_error_counter > 0) {
            --comm_error_counter;
        }
    }
    return retval;
}
