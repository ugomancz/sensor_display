/*
 * application.c
 *
 * This file serves as the entry point to the program.
 * It contains the main() function, interrupt handlers and a few
 * variables used throughout the project.
 *
 * Author: Ondrej Kostik
 */
#include "application.h"
#include "communication.h"
#include "gui.h"
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <string.h>

static channels_data ch_data = { 0 };
sensor_info current_sensor = { 0 };
sensor_info lookup_sensor = { 0 };
par_cnts old_par_cnts = { 0 };

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
    if (comm_state == MESSAGE_RECEIVED || comm_state == WAIT_TO_SEND) {
        return;
    }

    rx_buffer[rx_buffer_pos++] = UARTCharGet(UART6_BASE);
    /* Resets the T15 "message received" timer */
    if (comm_state != MESSAGE_RECEIVED) {
        TimerLoadSet(TIMER0_BASE, TIMER_A, T_15_CYCLES);
        TimerEnable(TIMER0_BASE, TIMER_A);
    }
}

/* Helper function to determine if click occurred inside of a button's coordinates */
static bool button_was_pressed(button *b, int32_t x, int32_t y) {
    return (b->coords.xMin <= x && b->coords.xMax >= x && b->coords.yMin <= y && b->coords.yMax >= y);
}

/* Touch screen interrupt handler */
int32_t touch_callback(uint32_t message, int32_t x, int32_t y) {
    if (message == MSG_PTR_UP) {
        if (to_menu_button.active && button_was_pressed(&to_menu_button, x, y)) {
            gui_context = MENU_GUI;
            TimerLoadSet(TIMER1_BASE, TIMER_A, FETCH_CH_VALUES_MSG_DELAY);
            TimerEnable(TIMER1_BASE, TIMER_A);
            comm_context = FETCH_CH_VALUES;
            ++clr_screen;
        } else if (lookup_accept_button.active && button_was_pressed(&lookup_accept_button, x, y)) {
            gui_context = MENU_GUI;
            TimerLoadSet(TIMER1_BASE, TIMER_A, FETCH_CH_VALUES_MSG_DELAY);
            TimerEnable(TIMER1_BASE, TIMER_A);
            comm_context = RESET_DOSE;
            memcpy(&current_sensor, &lookup_sensor, sizeof(current_sensor));
            memset(&old_par_cnts, 0, sizeof(old_par_cnts));
            comm_state = SEND_MESSAGE;
            ++clr_screen;
        } else if (lookup_reject_button.active && button_was_pressed(&lookup_reject_button, x, y)) {
            lookup_accept_button.active = false;
            lookup_reject_button.active = false;
            TimerEnable(TIMER1_BASE, TIMER_A);
            update_gui = true;
        } else if (to_measurements_button.active && button_was_pressed(&to_measurements_button, x, y)) {
            gui_context = MEASUREMENTS_GUI;
            ++clr_screen;
        } else if (to_lookup_button.active && button_was_pressed(&to_lookup_button, x, y)) {
            gui_context = SENSOR_LOOKUP_GUI;
            comm_context = SENSOR_LOOKUP;
            TimerLoadSet(TIMER1_BASE, TIMER_A, SENSOR_LOOKUP_MSG_DELAY);
            lookup_sensor.addr = 0x00;
            ++clr_screen;
        }
    }
    return 0;
}

/* Interrupt handler for the T15 "message received" timer */
void msg_received_timeout_handler() {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    comm_state = MESSAGE_RECEIVED;
}

/* Interrupt handler for the "send_message" timer */
void send_message_timeout_handler() {
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    if (comm_context != SENSOR_LOOKUP && comm_state == WAIT_TO_RECEIVE) {
        ++comm_error_counter;
    }
    comm_state = SEND_MESSAGE;
    if (comm_context == SENSOR_LOOKUP) {
        if (lookup_sensor.addr >= 247) {
            lookup_sensor.addr = 0;
        }
        ++lookup_sensor.addr;
    }
}

/*
 * Returns true if the par_cnt member of any of the currently fetched ch_val
 * structures is different when compared to the par_cnt of the last fetched data.
 */
static bool ch_pars_need_refresh(channels_data *ch_data) {
    if ((old_par_cnts.dose_rate != ch_data->dose_rate_val.par_cnt)
            || (old_par_cnts.dose != ch_data->dose_val.par_cnt)
            || (old_par_cnts.temp != ch_data->temp_val.par_cnt)) {
        return true;
    }
    return false;
}

/*
 * The entry point to the application.
 * Initialises all the required peripherals and begins execution with the
 * sensor lookup context. An infinite loop then handles the communication
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
     * Initialising with the SENSOR_LOOKUP_MSG_DELAY value because the first
     * context in which the sensor is after boot is the Sensor Lookup.
     */
    TimerLoadSet(TIMER1_BASE, TIMER_A, SENSOR_LOOKUP_MSG_DELAY);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

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
        if (comm_error_counter >= 10 && gui_context != ERROR_GUI) {
            TimerDisable(TIMER1_BASE, TIMER_A);
            gui_context = ERROR_GUI;
            comm_state = WAIT_TO_SEND;
            ++clr_screen;
        }
        /* Updating GUI if necessary */
        if (clr_screen > 0 || update_gui) {
            gui_update(&clr_screen, &ch_data, &current_sensor, &lookup_sensor);
            update_gui = false;
        }
        /* Communication control flow */
        switch (comm_state) {
        case SEND_MESSAGE:
            send_request();
            /*
             * The GUI is updated only in the sensor lookup context to draw
             * the currently probed address onto the display.
             */
            if (comm_context == SENSOR_LOOKUP) {
                update_gui = true;
            }
            comm_state = WAIT_TO_RECEIVE;
            break;
        case MESSAGE_RECEIVED:
            if (process_response(&ch_data, &old_par_cnts)) {
                ++comm_error_counter;
                comm_state = WAIT_TO_SEND;
                break;
            }
            /*
             * In the sensor lookup context, the "send message" timer gets
             * disabled here, because the actions following the "sensor found"
             * depend on the user's input.
             */
            if (comm_context == SENSOR_LOOKUP) {
                TimerDisable(TIMER1_BASE, TIMER_A);
                update_found_sensor_lookup_gui(&lookup_sensor);
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
            } else if (ch_pars_need_refresh(&ch_data)) {
                comm_context = FETCH_CH_PARS;
                comm_state = SEND_MESSAGE;
                break;
            } else {
                comm_context = FETCH_CH_VALUES;
                update_gui = true;
            }
            comm_state = WAIT_TO_SEND;
            break;
        }
    }
}
