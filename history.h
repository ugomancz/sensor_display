/*
 * history.h
 *
 *  Created on: 10 Apr 2022
 *      Author: ondra
 */

#ifndef HISTORY_H_
#define HISTORY_H_

/* History depth; new value is read every second so history with size 60 tracks last minute */
#define HISTORY_SIZE 60

extern float history_buffer[HISTORY_SIZE];
extern unsigned int history_buffer_pos;

/* Adds value to history. If there's no more unpopulated space, oldest value is replaced. */
void add_value_to_history(float value);

/* Returns smallest value in history. If history is empty, returns 0. */
float get_history_min();

/* Returns highest value in history. If history is empty, returns 0. */
float get_history_max();

/* Empties history */
void reset_history();

#endif /* HISTORY_H_ */
