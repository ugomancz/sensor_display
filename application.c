/*
 * application.c
 *
 * Author: Ondrej Kostik
 */
#include "application.h"
#include "communication.h"
#include "gui.h"
#include <ti/devices/msp432e4/driverlib/driverlib.h>

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

/* Interrupt handler for the T15 "message received" timer */
void msg_received_timeout_handler() {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    current_comm_state = MESSAGE_RECEIVED;
}

/* Interrupt handler for the "send_message" timer */
void send_message_timeout_handler() {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    current_comm_state = SEND_MESSAGE;
    if (current_comm_context == DEVICE_LOOKUP) {
        if (device_lookup_address >= 247) {
            device_lookup_address = 0;
        }
        ++device_lookup_address;
    }
}

/* Interrupt handler for the "request timeout" timer */
void no_response_timeout_handler() {
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    ++comm_error_counter;
}

/*
 * Returns true if the par_cnt member of any of the currently fetched ch_val
 * structures is different when compared to the par_cnt of the last fetched data.
 */
static bool ch_pars_need_refresh() {
    if ((last_par_cnts[DOSE_RATE_CH] != ch_values[DOSE_RATE_CH].par_cnt)
            || (last_par_cnts[DOSE_CH] != ch_values[DOSE_CH].par_cnt)
            || (last_par_cnts[TEMP_CH] != ch_values[TEMP_CH].par_cnt)) {
        return true;
    }
    return false;
}

/*
 * The entry point to the application.
 * Initialises all the required peripherals and begins execution with the
 * device lookup context. An infinite loop then handles the communication
 * with the target sensor and GUI events.
 */
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

    /* Initialise the "send message" timer */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {
    }
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerIntRegister(TIMER1_BASE, TIMER_A, send_message_timeout_handler);
    /*
     * Initialising with the DEVICE_LOOKUP_MSG_DELAY value because the first
     * context in which the device is after boot is the Device Lookup.
     */
    TimerLoadSet(TIMER1_BASE, TIMER_A, DEVICE_LOOKUP_MSG_DELAY);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    /* Initialise the "request timeout" timer */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2)) {
    }
    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
    TimerIntRegister(TIMER2_BASE, TIMER_A, no_response_timeout_handler);
    TimerLoadSet(TIMER2_BASE, TIMER_A, REQ_TIMEOUT_DELAY);
    IntEnable(INT_TIMER2A);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    /* Initialise the RX/TX buffers */
    if (init_comm_buffers()) {
        return -1;
    }

    /* Initialise the LCD and touch screen driver */
    Kentec_Init(ui32SysClock);
    TouchScreenInit(ui32SysClock);
    TouchScreenCallbackSet(touch_callback);
    Graphics_initContext(&g_context, &Kentec_GD, &Kentec_fxns);

    /* Start the "send message" timer */
    TimerEnable(TIMER1_BASE, TIMER_A);

    while (1) {
        /* Communication error handling */
        if (comm_error_counter >= 5 && current_gui_context != ERROR_GUI) {
            TimerDisable(TIMER1_BASE, TIMER_A);
            TimerDisable(TIMER2_BASE, TIMER_A);
            current_gui_context = ERROR_GUI;
            current_comm_state = WAIT_TO_SEND;
            ++clr_screen;
        }
        /* Updating GUI if necessary */
        if (clr_screen > 0 || update_gui) {
            gui_update();
        }
        /* Communication control flow */
        switch (current_comm_state) {
        case SEND_MESSAGE:
            send_request();
            /*
             * The GUI is updated only in the device lookup context to draw
             * the currently probed address onto the display. Similarly, the
             * "request timeout" timer is only started in the other contexts
             * as during the device lookup request timeouts are expected and
             * handled accordingly using the "send message" timer.
             */
            if (current_comm_context == DEVICE_LOOKUP) {
                update_gui = true;
            } else {
                TimerEnable(TIMER2_BASE, TIMER_A);
            }
            current_comm_state = WAIT_TO_RECEIVE;
            break;
        case MESSAGE_RECEIVED:
            /*
             * Since there's a message received, the "request timeout" timer
             * can be disabled and it's value reset.
             */
            TimerDisable(TIMER2_BASE, TIMER_A);
            TimerLoadSet(TIMER2_BASE, TIMER_A, REQ_TIMEOUT_DELAY);
            if (process_requested_data()) {
                ++comm_error_counter;
            }
            /*
             * In the device lookup context, the "send message" timer gets
             * disabled here, because the actions following the "device found"
             * depend on the user's input.
             */
            if (current_comm_context == DEVICE_LOOKUP) {
                TimerDisable(TIMER1_BASE, TIMER_A);
                update_found_device_lookup_gui();
                /*
                 * If the par_cnt members of fetched channel values changed
                 * compared to the last received data, the next requested data
                 * will be the channel parameters. Communication state is set to
                 * send the request immediately to prevent unnecessary waiting for
                 * the "send message" timer to trigger the request. If the par_cnt
                 * members haven't changed (or have just been updated), the device
                 * goes to reading only ch_val structures.
                 *
                 * Notice that GUI is only updated in the latter case. This is
                 * because the displayed units are based on the contents of ch_par
                 * structures. Therefore trying to show the correct unit without
                 * "knowing" what it is would cause an incorrect unit to be displayed.
                 */
            } else if (ch_pars_need_refresh()) {
                current_comm_context = FETCH_CH_PARS;
                current_comm_state = SEND_MESSAGE;
                break;
            } else {
                current_comm_context = FETCH_CH_VALUES;
                update_gui = true;
            }
            current_comm_state = WAIT_TO_SEND;
            break;
        }
    }
}
