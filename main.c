#include <stdio.h>
#include <stdlib.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/drivers/uart/UARTMSP432E4.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/pin_map.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include "globals.h"
#include "modbus_test.h"

volatile state current_state = SEND_MESSAGE;

/* Incoming byte at UART6 interrupt handler. */
void UART6_IRQHandler(void) {
    uint32_t ui32Status;

    // Get the interrupt status.
    ui32Status = UARTIntStatus(UART6_BASE, true);

    // Clear the asserted interrupts.
    UARTIntClear(UART6_BASE, ui32Status);

    if (ui32Status & UART_INT_TX) { // end-of-transmission interrupt
            set_direction(RECEIVE);
            return;
        }

    if (current_state != MESSAGE_RECEIVED) {
            // Resets the T15 (message_timeout) timer.
            TimerDisable(TIMER0_BASE, TIMER_A);
            // TimerEnable(TIMER0_BASE, TIMER_A);
    } else if (current_state == MESSAGE_RECEIVED) {
        // Should only occur in case of fault in transmission.
        return;
    }

    while (UARTCharsAvail(UART6_BASE)) {
        buffer[buffer_position++] = UARTCharGet(UART6_BASE);
    }
}

/* The interrupt handler for the T15 (message received) timer interrupt. */
void TIMER0A_IRQHandler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    current_state = MESSAGE_RECEIVED;
}

/* The interrupt handler for the periodic send message timer interrupt. */
void TIMER1A_IRQHandler(void) {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    current_state = SEND_MESSAGE;
}

int main(void) {
    uint32_t ui32SysClock;
    volatile uint32_t ui32Loop;

    // Run from the PLL at 120 MHz.
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
    FREQ);

    // Initialise LCD driver and touch driver.
    Kentec_Init(ui32SysClock);
    TouchScreenInit(ui32SysClock);
    Graphics_initContext(&sContext, &Kentec_GD, &Kentec_fxns);
    Graphics_setFont(&sContext, &g_sFontCmsc26);

    // Initialise UART 6.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
    IntMasterEnable();

    GPIOPinConfigure(GPIO_PP0_U6RX);
    GPIOPinConfigure(GPIO_PP1_U6TX);
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART6_BASE, ui32SysClock, BAUD_RATE,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_EVEN));
    IntEnable(INT_UART6);
    UARTIntEnable(UART6_BASE, UART_INT_RX /*| UART_INT_TX*/);
    //UARTTxIntModeSet(UART6_BASE, UART_TXINT_MODE_EOT);
    //UARTLoopbackEnable(UART6_BASE);

    // Initialise GPIO pin.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) {
    }
    GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_4);

    // Initialise the T15 (message received) timer.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // [test] TimerEnable(TIMER0_BASE, TIMER_A);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, FREQ/2);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    // [test] TimerEnable(TIMER1_BASE, TIMER_A);

    // Test communication.
    uint8_t data[] = { 0x00, 0x00, 0x00, 0x05 };
    frame frame = create_frame(0x01, 0x04, data, 0x3009);
    set_direction(TRANSMIT);
    if (!uart_send_frame(UART6_BASE, frame)) {
        // TODO: try here set_direction(RECEIVE);
        printf("Frame sent!\n");
    }

    while (1) {
        switch (current_state) {
        case SEND_MESSAGE:
            // TODO: send a message
            // TODO: current_state = IDLE;
            break;
        case MESSAGE_RECEIVED:
            // TODO: parse_incoming_message(buffer);
            // TODO: current_state = IDLE;
            break;
        default:
            continue;
        }
    }
}
