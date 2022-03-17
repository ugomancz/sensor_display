#include <communication.h>
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
#include "crc.h"
#include "mdg04.h"

volatile comm_states comm_state = SEND_MESSAGE;
volatile context_state current_context = FIND;
volatile context_state old_context = FIND;
volatile uint8_t device_id = 0x01;

/* UART interrupt handler */
void UART6_IRQHandler(void) {
    uint32_t ui32Status;
    TimerDisable(TIMER0_BASE, TIMER_A);

    // Get the interrupt status.
    ui32Status = UARTIntStatus(UART6_BASE, true);

    // Clear the asserted interrupts.
    UARTIntClear(UART6_BASE, ui32Status);

    // end-of-transmission interrupt
    if (ui32Status & UART_INT_TX) {
            set_direction(RECEIVE);
            return;
    }

    // Should only occur in case of fault in transmission
    if (comm_state == MESSAGE_RECEIVED) {
            return;
    }

    buffer[buffer_position++] = UARTCharGet(UART6_BASE);
    // Resets the T15 (message_timeout) timer
    if (comm_state != MESSAGE_RECEIVED) {
                TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
                TimerEnable(TIMER0_BASE, TIMER_A);
    }
}

/* The interrupt handler for the T15 (message received) timer interrupt */
void TIMER0A_IRQHandler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    comm_state = MESSAGE_RECEIVED;
}

/* TODO: Delete later */
void comm_test() {
    uint8_t data[] = { 0x00, 0x02, 0x00, 0x02 };
    frame f = create_frame(0x01, 0x04, data);
    set_direction(TRANSMIT);
    uart_send_frame(UART6_BASE, f);
}

int main(void) {
    uint32_t ui32SysClock;
    volatile uint32_t ui32Loop;

    // Run from the PLL at 120 MHz
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
    FREQ);

    // Initialise LCD driver and touch driver
    Kentec_Init(ui32SysClock);
    TouchScreenInit(ui32SysClock);
    Graphics_initContext(&sContext, &Kentec_GD, &Kentec_fxns);
    Graphics_setFont(&sContext, &g_sFontCmsc26);

    // Initialise UART 6
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

    // Initialise GPIO pin
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) {
    }
    GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_4);

    // Initialise the T15 (message received) timer
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Allocate the buffer
    if ((buffer = calloc(256, sizeof(uint8_t))) == NULL) {
        exit(EXIT_FAILURE);
    }

    // TODO: Define these
    frame read_dose, read_dose_rate, read_temp, read_id;

    // Test communication
    comm_test();

    while (1) {
        // Wait for the context shift to be complete
        if (current_context == old_context) {
            switch (comm_state) {
            case SEND_MESSAGE:
                // TODO: send a message
                // TODO: current_state = IDLE;
                break;
            case MESSAGE_RECEIVED:
                // TODO: parse_incoming_message(current_context);
                // TODO: current_state = IDLE;
                break;
            default:
                continue;
            }
        }
    }
}
