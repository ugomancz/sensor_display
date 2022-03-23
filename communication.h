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
#include "globals.h"

typedef enum {
    TRANSMIT,
    RECEIVE
} direction;

typedef enum {
    IDLE,
    SEND_MESSAGE,
    MESSAGE_RECEIVED
} comm_states;

typedef struct {
    uint8_t slave_address;
    uint8_t function_code;
    uint8_t *data;
    uint16_t data_length;
    uint16_t crc;
} frame;

extern volatile comm_states comm_state;

frame create_frame(uint8_t slave_address, uint8_t function_code, uint8_t *data);
void set_direction(direction dir);
int uart_send_frame(uint32_t uart_base, frame message);
bool parse_incoming_message(frame *f);

/* Top level function to send message to the sensor based on the current context */
void send_message();

/* Sends a specific message to the sensor. Used internally by send_message() */
void _get_dev_id();


/* Top level function to parse received messages based on the current context */
void parse_received();

void reset_buffer();


#endif /* COMMUNICATION_H_ */
