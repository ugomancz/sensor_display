/*
 * globals.h
 *
 *  Created on: 29 Jan 2022
 *      Author: ondra
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdbool.h>
#include "kentec.h"
#include "touch.h"
#include <ti/grlib/grlib.h>
#include "modbus_test.h"

#define FREQ 120000000
#define BAUD_RATE 9600
#define T_15 (1.5 * (11 * 1000000 / BAUD_RATE))
#define T_15_CYCLES (T_15 * (FREQ / 1000000))

typedef enum _state {WAITING, SEND_MESSAGE, MESSAGE_RECEIVED} state;

extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
Graphics_Context sContext;

volatile unsigned char buffer[256] = { { 0 } };
volatile unsigned int buffer_position = 0;



#endif /* GLOBALS_H_ */
