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

volatile bool interchar_timeout = false;
volatile bool interframe_timeout = false;

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

    /*// Resetting the T35 (interframe) timer.
    TimerDisable(TIMER1_BASE, TIMER_A);
    TimerEnable(TIMER1_BASE, TIMER_A);

    // Interchar delay checking.
    if (!interchar_timeout) {
        // Resets the T15 (interchar) timer.
        TimerDisable(TIMER0_BASE, TIMER_A);
        TimerEnable(TIMER0_BASE, TIMER_A);
    }
    else {
        // TODO Interchar delay error routine, clear interchar flag, clear buffer.
        return;
    }*/
    while (UARTCharsAvail(UART6_BASE)) {
        buffer[buffer_position++] = UARTCharGet(UART6_BASE);
    }
}

/* The interrupt handler for the T15 timer interrupt. */
void TIMER0A_IRQHandler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    interchar_timeout = true;
}

/* The interrupt handler for the T35 timer interrupt. */
void TIMER1A_IRQHandler(void) {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    interframe_timeout = true;
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
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);
    UARTTxIntModeSet(UART6_BASE, UART_TXINT_MODE_EOT);
    //UARTLoopbackEnable(UART6_BASE);

    // Initialise GPIO pin.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) {
    }
    GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_4);

    // Initialise T15 and T35 timers.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // T15
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // T35
    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER1_BASE, TIMER_A, T_35_CYCLES);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Test communication.
    uint8_t data[] = { 0x00, 0x00, 0x00, 0x05 };
    frame frame = create_frame(0x01, 0x04, data, 0x3009);
    set_direction(TRANSMIT);
    if (!uart_send_frame(UART6_BASE, frame)) {
        printf("Frame sent!\n");
    }

    // [test] TimerEnable(TIMER0_BASE, TIMER_A);
    // [test] TimerEnable(TIMER1_BASE, TIMER_A);

    while (1) {
        if (interframe_timeout) { // complete message received
            parse_incoming_message(buffer);
            interchar_timeout = false;
            interframe_timeout = false;
        }
    }
}
