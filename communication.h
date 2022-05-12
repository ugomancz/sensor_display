/*
 * communication.h
 *
 * This file contains declarations of variables representing the state
 * of the communication and functions used to interact with the communication
 * functionality.
 *
 * Author: Ondrej Kostik
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
} _comm_state;

/* Represents the current context of communication */
typedef enum {
    SENSOR_LOOKUP, RESET_DOSE, FETCH_CH_VALUES, FETCH_CH_PARS
} _comm_context;

/* Counter to keep track of request timeouts */
extern volatile uint8_t comm_error_counter;

/* Tracks the current context of communication */
extern volatile _comm_context comm_context;

/* Tracks the current state of communication */
extern volatile _comm_state comm_state;

/* RX buffer serve as a bridge between the ModBus library and the UART communication. */
extern uint8_t *rx_buffer;
extern volatile uint8_t rx_buffer_pos;

/* Sets the direction of communication on the serial line */
void set_comm_direction(direction dir);

/* Allocates the RX and TX buffers */
int init_comm_buffers();

/* Sends the appropriate request to the sensor based on the current communication context */
void send_request();

/* Parses the data returned from the sensor based on the current communication context */
int process_response(channels_data *ch_data, par_cnts *old_par_cnts);

#endif /* COMMUNICATION_H_ */
