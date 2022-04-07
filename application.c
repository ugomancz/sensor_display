#include "application.h"
#include <stdlib.h>
#include <string.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/pin_map.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/devices/msp432e4/inc/msp432e411y.h>
#include <ti/drivers/uart/UARTMSP432E4.h>
#include "communication.h"
#include "crc.h"
#include "gui.h"
#include "mdg04.h"

volatile comm_states comm_state = SEND_MESSAGE;
volatile context_state current_context = FIND;
volatile context_state old_context = FIND;
volatile uint8_t device_address = 0x01;
uint8_t *buffer;
uint16_t buffer_position = 0;
dev_id device_id = {0};
ch_val ch_value = {0};

void TIMER1A_IRQHandler();

void context_switch_done() {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    reset_buffer();
    memset(&ch_value, 0, sizeof(ch_value));
    UARTIntClear(UART6_BASE, UARTIntStatus(UART6_BASE, true));
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_TX);
    old_context = current_context;
    comm_state = SEND_MESSAGE;
    TimerDisable(TIMER1_BASE, TIMER_A);
    TimerIntRegister(TIMER1_BASE, TIMER_A, TIMER1A_IRQHandler);
    if (current_context == FIND) {
        TimerLoadSet(TIMER1_BASE, TIMER_A, FIND_SEND_MSG_DELAY);
    } else {
        TimerLoadSet(TIMER1_BASE, TIMER_A, SEND_MSG_DELAY);
    }
    TimerEnable(TIMER1_BASE, TIMER_A);
}

void start_context_switch() {
    TimerDisable(TIMER1_BASE, TIMER_A);
    UARTIntDisable(UART6_BASE, UART_INT_RX | UART_INT_TX);
    TimerIntRegister(TIMER1_BASE, TIMER_A, context_switch_done);
    TimerLoadSet(TIMER1_BASE, TIMER_A, CONTEXT_SWITCH_DELAY);
    TimerEnable(TIMER1_BASE, TIMER_A);
}

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

/* The interrupt handler for the "send_message" timer interrupt */
void TIMER1A_IRQHandler(void) {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    comm_state = SEND_MESSAGE;
    if (current_context == FIND) {
        if (device_address >= 247) {
            device_address = 0;
        }
        ++device_address;
    }
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
    TouchScreenCallbackSet(touchcallback);
    Graphics_initContext(&g_context, &Kentec_GD, &Kentec_fxns);

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

    // Initialise the "send_message" timer
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, FIND_SEND_MSG_DELAY);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Allocate the buffer
    if ((buffer = calloc(256, sizeof(uint8_t))) == NULL) {
        exit(EXIT_FAILURE);
    }

    // Start GUI
    update_display();

    // Start periodic "send_message" timer
    TimerEnable(TIMER1_BASE, TIMER_A);

    while (1) {
        // Wait for the context shift to be complete
        if (current_context == old_context) {
            switch (comm_state) {
            case SEND_MESSAGE:
                send_message();
                update_display();
                comm_state = IDLE;
                break;
            case MESSAGE_RECEIVED:
                if (current_context == FIND) {
                    TimerDisable(TIMER1_BASE, TIMER_A);
                }
                parse_received();
                update_display();
                comm_state = IDLE;
                break;
            default:
                continue;
            }
        }
    }
}
