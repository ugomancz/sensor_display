/*
 * modbus_def.h
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TRANSMIT,
    RECEIVE
} direction;

typedef enum {
    IDLE,
    SEND_MESSAGE,
    MESSAGE_RECEIVED
} comm_states;

extern volatile comm_states comm_state;

void set_direction(direction dir);

/* Top level function to send message to the sensor based on the current context */
void send_message();

/* Sends a specific message to the sensor. Used internally by send_message() */
void _get_dev_id();


/* Top level function to parse received messages based on the current context */
void parse_received();

void reset_buffer();


#endif /* COMMUNICATION_H_ */
