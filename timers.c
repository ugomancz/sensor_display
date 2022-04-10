/*
 * timers.c
 *
 *  Created on: 10 Apr 2022
 *      Author: ondra
 */
#include "timers.h"
#include "application.h"
#include "communication.h"
#include "gui.h"
#include "touch.h"
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <string.h>

void context_switch_timeout_handler() {
    TimerDisable(TIMER1_BASE, TIMER_A);
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    reset_buffer();
    memset(&ch_value, 0, sizeof(ch_value));
    UARTIntClear(UART6_BASE, UARTIntStatus(UART6_BASE, true));

    old_context = current_context;
    /* If context was switched to FIND, device address and device_id structure are reset to initial values */
    if (current_context == FIND) {
        device_address = 0x01;
        memset(&device_id, 0, sizeof(dev_id));
        TimerLoadSet(TIMER1_BASE, TIMER_A, FIND_SEND_MSG_DELAY);
    } else {
        TimerLoadSet(TIMER1_BASE, TIMER_A, SEND_MSG_DELAY);
    }
    TimerIntRegister(TIMER1_BASE, TIMER_A, send_msg_timeout_handler);
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_TX);
    UARTRxErrorClear(UART6_BASE);
    TouchScreenCallbackSet(touch_callback);
    comm_state = SEND_MESSAGE;
    TimerEnable(TIMER1_BASE, TIMER_A);
}

void send_msg_timeout_handler() {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    comm_state = SEND_MESSAGE;
    if (current_context == FIND) {
        if (device_address >= 247) {
            device_address = 0;
        }
        ++device_address;
    }
}

void msg_received_timeout_handler() {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    comm_state = MESSAGE_RECEIVED;
}
