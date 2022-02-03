/*
 * modbus_def.h
 *
 *  Created on: 26 Jan 2022
 *      Author: ondra
 */

#ifndef MODBUS_TEST_H_
#define MODBUS_TEST_H_

#include <stdint.h>

typedef enum _direction {
    TRANSMIT,
    RECEIVE
} direction;

typedef struct _frame {
    uint8_t slave_address;
    uint8_t function_code;
    uint8_t *data;
    uint8_t data_length;
    uint16_t crc;
} frame;

volatile direction de_nre;


frame create_frame(uint8_t slave_address, uint8_t function_code, uint8_t data[], uint16_t crc);
int uart_send_frame(uint32_t uart_base, frame message);
void set_direction(direction dir);

frame parse_incoming_message();

#endif /* MODBUS_TEST_H_ */
