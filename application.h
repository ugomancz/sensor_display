/*
 * application.c
 *
 * Author: Ondrej Kostik
 */
#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <stdbool.h>
#include <stdint.h>

/* CPU frequency in Hz. */
#define FREQ 120000000

/* UART baud rate used for communication with the sensor. */
#define BAUD_RATE 9600

/* Time it takes to transfer 1.5 characters over UART in CPU cycles. */
#define T_15_CYCLES ((1.5 * (11 * 1000000 / BAUD_RATE)) * (FREQ / 1000000))

/* Delay between fetching new channel data from the sensor in CPU cycles. */
#define FETCH_CH_VALUES_MSG_DELAY FREQ

/* Delay between probing addresses in device lookup context in CPU cycles. */
#define DEVICE_LOOKUP_MSG_DELAY FREQ/4

/* Timeout value for the "request timeout" timer in CPU cycles. */
#define REQ_TIMEOUT_DELAY FREQ

#endif /* APPLICATION_H_ */
