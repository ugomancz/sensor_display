/*
 * globals.h
 *
 *  Created on: 29 Jan 2022
 *      Author: ondra
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdbool.h>
#include <stdint.h>



#define FREQ 120000000
#define BAUD_RATE 9600
#define T_15 (1.5 * (11 * 1000000 / BAUD_RATE))
#define T_15_CYCLES (T_15 * (FREQ / 1000000))

typedef enum {IDLE, SEND_MESSAGE, MESSAGE_RECEIVED} comm_states;
typedef enum {FIND, MENU, DOSE, DOSE_RATE} context_state;

extern uint8_t *buffer;
extern uint16_t buffer_position;
extern volatile uint8_t device_id;

#endif /* GLOBALS_H_ */
