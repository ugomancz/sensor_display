/*
 * application.c
 *
 * Author: Ondrej Kostik
 */
#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "vf_standard.h"
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

/* Structure encapsulating all the values and parameters of a set of channels */
typedef struct {
    ch_val dose_rate_val;
    ch_val dose_val;
    ch_val temp_val;
    ch_par dose_rate_par;
    ch_par dose_par;
    ch_par temp_par;
} channels_data;

/*
 * Structure encapsulating par_cnt values of all three channels.
 * Used to keep track of changes.
 */
typedef struct {
    uint16_t dose_rate;
    uint16_t dose;
    uint16_t temp;
} par_cnts;

/* Structure encapsulating basic info about a sensor */
typedef struct {
    uint8_t addr;
    dev_id id;
} sensor_info;

extern sensor_info current_sensor;
extern sensor_info lookup_sensor;

/* Tracks whether the whole screen should be cleared before drawing new GUI elements.
 * Typically happens when GUI context is switched.
 */
extern volatile uint8_t clr_screen;

/* Flag set to true if a GUI update is required */
extern volatile bool update_gui;

#endif /* APPLICATION_H_ */
