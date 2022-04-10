/*
 * history.c
 *
 *  Created on: 10 Apr 2022
 *      Author: ondra
 */
#include "history.h"
#include <float.h>
#include <string.h>

float history_buffer[HISTORY_SIZE] = { 0 };
unsigned int history_buffer_pos = 0;

void add_value_to_history(float value) {
    history_buffer[history_buffer_pos++ % HISTORY_SIZE] = value;
}

float get_history_min() {
    float min = FLT_MAX;
    for (short i = 0; i < HISTORY_SIZE; ++i) {
        if (history_buffer[i] != 0 && min > history_buffer[i]) {
            min = history_buffer[i];
        }
    }
    return min == FLT_MAX ? 0 : min;
}

float get_history_max() {
    float max = FLT_MIN;
    for (short i = 0; i < HISTORY_SIZE; ++i) {
        if (history_buffer[i] != 0 && max < history_buffer[i]) {
            max = history_buffer[i];
        }
    }
    return max == FLT_MIN ? 0 : max;
}

void reset_history() {
    memset(history_buffer, 0, HISTORY_SIZE);
    history_buffer_pos = 0;
}
