/*
 * application.h
 *
 *  Created on: 4 Apr 2022
 *      Author: ondra
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <stdbool.h>
#include <stdint.h>
#include "mdg04.h"



#define FREQ 120000000
#define BAUD_RATE 9600
#define T_15_CYCLES ((1.5 * (11 * 1000000 / BAUD_RATE)) * (FREQ / 1000000))
#define SEND_MSG_DELAY FREQ
#define FIND_SEND_MSG_DELAY FREQ/5
#define CONTEXT_SWITCH_DELAY FREQ/5


typedef enum {FIND, MENU, DOSE, DOSE_RATE} context_state;

extern volatile context_state current_context;
extern uint8_t *buffer;
extern uint16_t buffer_position;
extern volatile uint8_t device_address;
extern dev_id device_id;
extern ch_val ch_value;

void start_context_switch();
void context_switch_done();

#endif /* APPLICATION_H_ */
