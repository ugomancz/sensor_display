/*
 * application.c
 *
 *  Created on: 05 Jan 2022
 *      Author: ondra
 */
#include "application.h"
#include "communication.h"
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/pin_map.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <ti/drivers/uart/UARTMSP432E4.h>
#include <stdlib.h>

/* UART RX/TX interrupt handler */
void uart_int_handler(void) {
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
    if (current_comm_state == MESSAGE_RECEIVED || current_comm_state == WAIT_TO_SEND) {
        return;
    }

    rx_buffer[rx_buffer_pos++] = UARTCharGet(UART6_BASE);
    /* Resets the T15 "message received" timer */
    if (current_comm_state != MESSAGE_RECEIVED) {
        TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
        TimerEnable(TIMER0_BASE, TIMER_A);
    }
}

void msg_received_timeout_handler() {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    current_comm_state = MESSAGE_RECEIVED;
}

int main(void) {
    uint32_t ui32SysClock;
    volatile uint32_t ui32Loop;

    /* Run from the PLL at 120 MHz */
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
    FREQ);

    IntMasterEnable();

    /* Initialise and set up the UART 6 peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART6)) {
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP)) {
    }

    GPIOPinConfigure(GPIO_PP0_U6RX);
    GPIOPinConfigure(GPIO_PP1_U6TX);
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART6_BASE, ui32SysClock, BAUD_RATE,
            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_EVEN));
    UARTIntRegister(UART6_BASE, uart_int_handler);
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
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {
    }
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, msg_received_timeout_handler);
    TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /* Initialise the RX/TX buffers */
    if (init_comm_buffers()) {
        exit(-1);
    }

    request_current_channel_values();

    while (1) {
    }
}
