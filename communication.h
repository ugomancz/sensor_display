/*
 * communication.h
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */
#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include "mdg04.h"

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

/* Represents specific channel values in the ch_values array */
typedef enum {
    TEMP_CH = 0, DOSE_CH = 1, DOSE_RATE_CH = 2
} ch_type;

/* Holds the address of the "communicated to" device */
extern volatile uint8_t device_address;

/* Tracks the current state of communication */
extern volatile comm_state current_comm_state;

/* RX and TX buffers serve as a bridge between the ModBus library and the UART communication. */
extern uint8_t *tx_buffer;
extern uint8_t *rx_buffer;
extern volatile uint8_t tx_buffer_pos;
extern volatile uint8_t rx_buffer_pos;

/* Array of ch_val structures to hold the channel value data of all three channels */
extern ch_val ch_values[3];

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

/* Sends a request to the sensor for all three fast channel ch_val structures */
void request_current_channel_values();

#endif /* COMMUNICATION_H_ */
