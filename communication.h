/*
 * communication.h
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */
#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include "application.h"

#define TX_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 256

/* Represents the direction of communication */
typedef enum {
    TRANSMIT, RECEIVE
} direction;

/* Represents the current state of communication */
typedef enum {
    WAIT_TO_SEND, WAIT_TO_RECEIVE, SEND_MESSAGE, MESSAGE_RECEIVED
} comm_state;

/* Represents the current context of communication */
typedef enum {
    DEVICE_LOOKUP, FETCH_CH_VALUES, FETCH_CH_PARS
} comm_context;

/* Counter to keep track of request timeouts */
extern volatile uint8_t comm_error_counter;

/* Tracks the current context of communication */
extern volatile comm_context current_comm_context;

/* Tracks the current state of communication */
extern volatile comm_state current_comm_state;

/* RX and TX buffers serve as a bridge between the ModBus library and the UART communication. */
extern uint8_t *tx_buffer;
extern uint8_t *rx_buffer;
extern volatile uint8_t tx_buffer_pos;
extern volatile uint8_t rx_buffer_pos;

/* Sets the direction of communication on the serial line */
void set_comm_direction(direction dir);

/* Allocates the RX and TX buffers */
int init_comm_buffers();

/* Resets TX buffer */
void reset_tx_buffer();

/* Resets RX buffer */
void reset_rx_buffer();

/* Sends the data in TX buffer to the UART */
void uart_send();

/* Sends the appropriate request to the sensor based on the current communication context */
void send_request();

/* Parses the data returned from the sensor based on the current communication context */
int process_requested_data(channels_data *ch_data, par_cnts * old_par_cnts);

#endif /* COMMUNICATION_H_ */
