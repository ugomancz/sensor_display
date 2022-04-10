/*
 * communication.h
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */
#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

typedef enum {
    TRANSMIT, RECEIVE
} direction;

typedef enum {
    WAIT_TO_SEND, WAIT_TO_RECEIVE, SEND_MESSAGE, MESSAGE_RECEIVED
} _comm_state;

/* Tracks the current state of communication */
extern volatile _comm_state comm_state;

/* Sets the direction of communication on the serial line */
void set_comm_direction(direction dir);

/* Top level function to send a message to the sensor based on the current context */
void send_message();

/* Sends a specific message to the sensor. Used internally by send_message() */
void _get_dev_id();
void _get_dev_temp();
void _get_dev_dose();
void _get_dev_dose_rate();

/* Parses received messages based on the current context */
void parse_received();

/* Resets the receive buffer */
void reset_buffer();

#endif /* COMMUNICATION_H_ */
