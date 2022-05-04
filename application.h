/*
 * application.c
 *
 *  Created on: 05 Jan 2022
 *      Author: ondra
 */
#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <stdbool.h>
#include <stdint.h>

#define FREQ 120000000
#define BAUD_RATE 9600
#define T_15_CYCLES ((1.5 * (11 * 1000000 / BAUD_RATE)) * (FREQ / 1000000))
#define FETCH_CH_VALUES_MSG_DELAY FREQ
#define DEVICE_LOOKUP_MSG_DELAY FREQ/4
#define COMM_ERROR_DELAY FREQ

#endif /* APPLICATION_H_ */
