/*
 * timers.h
 *
 *  Created on: 10 Apr 2022
 *      Author: ondra
 */

#ifndef TIMERS_H_
#define TIMERS_H_

/* An interrupt handler for the "send_message" timer interrupt */
void send_msg_timeout_handler();

/* An interrupt handler for the "context switch" timer interrupt */
void context_switch_timeout_handler();

/* An interrupt handler for the T15 "message received" timer interrupt */
void msg_received_timeout_handler();

#endif /* TIMERS_H_ */
