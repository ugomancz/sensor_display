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

typedef struct {
    uint8_t slave_address;
    uint8_t function_code;
    uint8_t *data;
    uint16_t data_length;
    uint16_t crc;
} frame;

frame create_frame(uint8_t slave_address, uint8_t function_code, uint8_t *data);
void set_direction(direction dir);
int uart_send_frame(uint32_t uart_base, frame message);
bool parse_incoming_message(frame *f);

#endif /* COMMUNICATION_H_ */
