/*
 * application.c
 *
 *  Created on: 05 Jan 2022
 *      Author: ondra
 */
#include "application.h"
#include "communication.h"
#include "crc.h"
#include "gui.h"
#include "timers.h"
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/pin_map.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <ti/drivers/uart/UARTMSP432E4.h>
#include <stdlib.h>
#include <string.h>

volatile _comm_state comm_state = SEND_MESSAGE;
volatile context_state current_context = FIND;
volatile context_state old_context = FIND;
volatile uint8_t device_address = 0x01;
uint8_t *buffer;
volatile uint16_t buffer_position = 0;
dev_id device_id = { 0 };
ch_val ch_value = { 0 };

/* UART 6 RX/TX interrupt handler */
void UART6_IRQHandler(void) {
    uint32_t ui32Status;
    TimerDisable(TIMER0_BASE, TIMER_A);

    /* Get the interrupt status */
    ui32Status = UARTIntStatus(UART6_BASE, true);

    /* Clear the asserted interrupts */
    UARTIntClear(UART6_BASE, ui32Status);

    /* EOT (end-of-transmission) interrupt only changes the direction of communication */
    if (ui32Status & UART_INT_TX) {
        set_comm_direction(RECEIVE);
        return;
    }

    /* Should only occur in case of a fault in a transmission */
    if (comm_state == MESSAGE_RECEIVED || comm_state == WAIT_TO_SEND) {
        return;
    }

    buffer[buffer_position++] = UARTCharGet(UART6_BASE);
    /* Resets the T15 "message received" timer */
    if (comm_state != MESSAGE_RECEIVED) {
        TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
        TimerEnable(TIMER0_BASE, TIMER_A);
    }
}

/* Initialise a context switch */
void start_context_switch() {
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerDisable(TIMER1_BASE, TIMER_A);
    UARTIntDisable(UART6_BASE, UART_INT_RX | UART_INT_TX);
    TouchScreenCallbackSet(0);
    comm_state = WAIT_TO_SEND;

    TimerIntRegister(TIMER1_BASE, TIMER_A, context_switch_timeout_handler);
    TimerLoadSet(TIMER1_BASE, TIMER_A, CONTEXT_SWITCH_DELAY);
    TimerEnable(TIMER1_BASE, TIMER_A);
}

int main(void) {
    uint32_t ui32SysClock;
    volatile uint32_t ui32Loop;

    /* Run from the PLL at 120 MHz */
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
    FREQ);

    /* Initialise the LCD and touch screen driver */
    Kentec_Init(ui32SysClock);
    TouchScreenInit(ui32SysClock);
    TouchScreenCallbackSet(touch_callback);
    Graphics_initContext(&g_context, &Kentec_GD, &Kentec_fxns);

    /* Initialise and set up the UART 6 peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
    IntMasterEnable();

    GPIOPinConfigure(GPIO_PP0_U6RX);
    GPIOPinConfigure(GPIO_PP1_U6TX);
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART6_BASE, ui32SysClock, BAUD_RATE,
            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_EVEN));
    IntEnable(INT_UART6);
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_TX);
    UARTFIFODisable(UART6_BASE);
    UARTTxIntModeSet(UART6_BASE, UART_TXINT_MODE_EOT);

    /* Initialise the GPIO PH4 pin */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) {
    }
    GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_4);

    /* Initialise the T15 "message received" timer */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, msg_received_timeout_handler);
    TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /* Initialise the "send message" timer */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerIntRegister(TIMER1_BASE, TIMER_A, send_msg_timeout_handler);
    TimerLoadSet(TIMER1_BASE, TIMER_A, FIND_SEND_MSG_DELAY);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    /* Allocate the buffer for incoming data */
    if ((buffer = calloc(RX_BUFFER_SIZE, sizeof(uint8_t))) == NULL) {
        exit(EXIT_FAILURE);
    }

    display_update();
    TimerEnable(TIMER1_BASE, TIMER_A);

    while (1) {
        /* Nothing happens during a context switch */
        if (current_context == old_context) {
            switch (comm_state) {
            case SEND_MESSAGE:
                send_message();
                display_update();
                comm_state = WAIT_TO_RECEIVE;
                break;
            case MESSAGE_RECEIVED:
                if (current_context == FIND) {
                    /* Waiting for the user's input */
                    TimerDisable(TIMER1_BASE, TIMER_A);
                }
                parse_received();
                display_update();
                comm_state = WAIT_TO_SEND;
                break;
            }
        }
    }
}
