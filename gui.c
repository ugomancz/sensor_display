/*
 * gui.c
 *
 *  Created on: 1 May 2022
 *      Author: ondra
 */
#include "gui.h"
#include "application.h"
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <stdio.h>
#include <string.h>

/* Structures used by the display driver */
extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
extern Graphics_Context g_context;

volatile gui_context current_gui_context = DEVICE_LOOKUP_GUI;
volatile bool clr_screen = true;
volatile bool update_gui = true;
float last_displayed_values[3] = { 0 };

/* "Menu" button in top right corner */
button to_menu_button = {
        .coords = { .xMin = 248, .yMin = 2, .xMax = 318, .yMax = 37 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm22b,
        .text = "Menu",
        .active = false
};

/* "Yes" button within DEVICE_LOOKUP_GUI context */
button find_accept_button = {
        .coords = { .xMin = 50, .yMin = 180, .xMax = 150, .yMax = 220 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm24b,
        .text = "Yes",
        .active = false
};

/* "No" button within DEVICE_LOOKUP_GUI context */
button find_reject_button = {
        .coords = { .xMin = 170, .yMin = 180, .xMax = 270, .yMax = 220 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm24b,
        .text = "No",
        .active = false
};

/* "Dose" button within MENU_GUI context */
button menu_dose_button = {
        .coords = { .xMin = 4, .yMin = 180, .xMax = 102, .yMax = 220 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm20b,
        .text = "Dose",
        .active = false
};

/* "Dose Rate" button within MENU_GUI context */
button menu_dose_rate_button = {
        .coords = { .xMin = 110, .yMin = 180, .xMax = 208, .yMax = 220 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm20b,
        .text = "Dose Rate",
        .active = false
};

/* "Find Device" button within MENU_GUI context */
button menu_find_button = {
        .coords = { .xMin = 216, .yMin = 180, .xMax = 316, .yMax = 220 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm18b,
        .text = "Dev. lookup",
        .active = false
};

/* Helper function to determine if click occurred inside of a button's coordinates */
bool button_was_pressed(button *b, int32_t x, int32_t y) {
    return (b->coords.xMin <= x && b->coords.xMax >= x && b->coords.yMin <= y && b->coords.yMax >= y);
}

/* Touch screen interrupt handler */
int32_t touch_callback(uint32_t message, int32_t x, int32_t y) {
    if (message == MSG_PTR_UP) {
        gui_context initial_gui_context = current_gui_context;
        if (to_menu_button.active && button_was_pressed(&to_menu_button, x, y)) {
            current_gui_context = MENU_GUI;
            TimerLoadSet(TIMER1_BASE, TIMER_A, FETCH_CH_VALUES_MSG_DELAY);
            TimerEnable(TIMER1_BASE, TIMER_A);
            current_comm_context = FETCH_CH_VALUES;
        } else if (find_accept_button.active && button_was_pressed(&find_accept_button, x, y)) {
            current_gui_context = MENU_GUI;
            TimerLoadSet(TIMER1_BASE, TIMER_A, FETCH_CH_VALUES_MSG_DELAY);
            TimerEnable(TIMER1_BASE, TIMER_A);
            current_comm_context = FETCH_CH_VALUES;
            device_address = device_lookup_address;
            device_id = device_lookup_id;
        } else if (find_reject_button.active && button_was_pressed(&find_reject_button, x, y)) {
            find_accept_button.active = false;
            find_reject_button.active = false;
            ++device_lookup_address;
            TimerEnable(TIMER1_BASE, TIMER_A);
        } else if (menu_dose_button.active && button_was_pressed(&menu_dose_button, x, y)) {
            current_gui_context = DOSE_GUI;
        } else if (menu_dose_rate_button.active && button_was_pressed(&menu_dose_rate_button, x, y)) {
            current_gui_context = DOSE_RATE_GUI;
        } else if (menu_find_button.active && button_was_pressed(&menu_find_button, x, y)) {
            current_gui_context = DEVICE_LOOKUP_GUI;
            current_comm_context = DEVICE_LOOKUP;
            device_lookup_address = 0x01;
            TimerLoadSet(TIMER1_BASE, TIMER_A, DEVICE_LOOKUP_MSG_DELAY);
        }
        if (initial_gui_context != current_gui_context) {
            menu_dose_button.active = false;
            menu_dose_rate_button.active = false;
            menu_find_button.active = false;
            find_accept_button.active = false;
            find_reject_button.active = false;
            to_menu_button.active = false;
            clr_screen = true;
            current_comm_state = SEND_MESSAGE;
        }
    }
    return 0;
}

/* Draws a button onto the display */
void draw_button(button *b) {
    b->active = true;
    Graphics_setForegroundColor(&g_context, b->button_color);
    Graphics_fillRectangle(&g_context, &(b->coords));
    Graphics_setForegroundColor(&g_context, b->text_color);
    Graphics_setFont(&g_context, b->font);
    Graphics_drawStringCentered(&g_context, b->text, -1, (b->coords.xMax + b->coords.xMin) / 2,
            (b->coords.yMax + b->coords.yMin) / 2, false);
}

/* Top level function which updates the screen contents based on the current GUI context */
void gui_update() {
    update_gui = false;
    if (clr_screen) {
        Graphics_clearDisplay(&g_context);
    }
    switch (current_gui_context) {
    case DEVICE_LOOKUP_GUI:
        if (clr_screen) {
            _init_device_lookup_gui();
        } else {
            _update_device_lookup_gui();
        }
        break;
    case MENU_GUI:
        if (clr_screen) {
            _init_menu_gui();
        } else {
            _update_menu_gui();
        }
        break;
    case DOSE_GUI:
        if (clr_screen) {
            _init_dose_gui();
        } else {
            _update_dose_gui();
        }
        break;
    case DOSE_RATE_GUI:
        if (clr_screen) {
            _init_dose_rate_gui();
        } else {
            _update_dose_rate_gui();
        }
        break;
    }
    clr_screen = false;
}

void _init_device_lookup_gui() {
    char string_buffer[10] = { 0 };
    Graphics_setFont(&g_context, &g_sFontCm24b);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);

    Graphics_drawString(&g_context, "Device lookup", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);

    Graphics_setFont(&g_context, &g_sFontCm22b);
    Graphics_drawString(&g_context, "Scanning address:", -1, 4, 60, false);

    sprintf(string_buffer, "0x%02x", device_lookup_address);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 200, 60, false);

    draw_button(&to_menu_button);
}

void update_found_device_lookup_gui() {
    char string_buffer[25] = { 0 };
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm22b);
    Graphics_drawStringCentered(&g_context, "Device found!", -1, 160, 98, false);

    Graphics_setFont(&g_context, &g_sFontCm20);

    sprintf(string_buffer, "Name: %s", device_lookup_id.pr_name);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 120, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "HW ID: ");
    format_hw_id(string_buffer, device_lookup_id.hw_id);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 140, false);

    draw_button(&find_accept_button);
    draw_button(&find_reject_button);
}

void _update_device_lookup_gui() {
    const Graphics_Rectangle hide_address = { .xMin = 200, .yMin = 55, .xMax = 280, .yMax = 80 };
    const Graphics_Rectangle hide_found = { .xMin = 0, .yMin = 85, .xMax = 319, .yMax = 239 };
    char string_buffer[10] = { 0 };

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&g_context, &hide_address);

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    sprintf(string_buffer, "0x%02x", device_lookup_address);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 200, 60, false);
    find_accept_button.active = false;
    find_reject_button.active = false;
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&g_context, &hide_found);
}

void _init_menu_gui() {
    char string_buffer[25] = { 0 };
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm24b);

    Graphics_drawString(&g_context, "Menu", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);

    Graphics_setFont(&g_context, &g_sFontCm20);

    sprintf(string_buffer, "Address: 0x%02x", device_address);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 50, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "SN: %d", device_id.ser_no);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 70, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "HW ID: ");
    format_hw_id(string_buffer, device_id.hw_id);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 90, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "SW ID: ");
    format_sw_id(string_buffer, device_id.sw_id);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 110, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "SW Ver.: %02d.%02d", (device_id.sw_ver >> 8), (uint8_t) device_id.sw_ver);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 130, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "Temp: %0.2f deg. C", ch_values[TEMP_CH].val);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 150, false);
    last_displayed_values[TEMP_CH] = ch_values[TEMP_CH].val;

    draw_button(&menu_dose_button);
    draw_button(&menu_dose_rate_button);
    draw_button(&menu_find_button);
}

void _update_menu_gui() {
    if (ch_values[TEMP_CH].val != last_displayed_values[TEMP_CH]) {
        const Graphics_Rectangle hide_temp = { .xMin = 60, .yMin = 150, .xMax = 170, .yMax = 175 };
        char string_buffer[20] = { 0 };

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_temp);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_setFont(&g_context, &g_sFontCm20);

        sprintf(string_buffer, "%0.2f deg. C", ch_values[TEMP_CH].val);
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 70, 150, false);
        last_displayed_values[TEMP_CH] = ch_values[TEMP_CH].val;
    }
}

void _init_dose_gui() {
    char string_buffer[20] = { 0 };
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm24b);

    Graphics_drawString(&g_context, "Dose", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);

    Graphics_setFont(&g_context, &g_sFontCm48b);

    sprintf(string_buffer, "%0.2e", ch_values[DOSE_CH].val);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 60, false);

    Graphics_setFont(&g_context, &g_sFontCm32b);
    Graphics_drawString(&g_context, "Gy", -1, 205, 60, false);

    Graphics_setFont(&g_context, &g_sFontCm22b);
    Graphics_drawString(&g_context, "Last minute extremes:", -1, 4, 120, false);

    Graphics_setFont(&g_context, &g_sFontCm20b);

    memset(string_buffer, 0, 20);
    //sprintf(string_buffer, "%0.2e", get_history_min());
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 58, 155, false);
    Graphics_drawString(&g_context, "Min:", -1, 4, 155, false);
    Graphics_drawString(&g_context, "Gy", -1, 150, 155, false);

    memset(string_buffer, 0, 20);
    //sprintf(string_buffer, "%0.2e", get_history_max());
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 58, 180, false);
    Graphics_drawString(&g_context, "Max:", -1, 4, 180, false);
    Graphics_drawString(&g_context, "Gy", -1, 150, 180, false);

    draw_button(&to_menu_button);
    last_displayed_values[DOSE_CH] = ch_values[DOSE_CH].val;
}

void _update_dose_gui() {
    if (last_displayed_values[DOSE_CH] != ch_values[DOSE_CH].val) {
        const Graphics_Rectangle hide_current = { .xMin = 4, .yMin = 45, .xMax = 204, .yMax = 110 };
        const Graphics_Rectangle hide_min_max = { .xMin = 55, .yMin = 145, .xMax = 149, .yMax = 210 };
        char string_buffer[20] = { 0 };

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_current);
        Graphics_fillRectangle(&g_context, &hide_min_max);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_setFont(&g_context, &g_sFontCm48b);

        sprintf(string_buffer, "%0.2e", ch_values[DOSE_CH].val);
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 60, false);

        Graphics_setFont(&g_context, &g_sFontCm20b);

        memset(string_buffer, 0, 20);
        //sprintf(string_buffer, "%0.2e", get_history_min());
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 58, 155, false);

        memset(string_buffer, 0, 20);
        //sprintf(string_buffer, "%0.2e", get_history_max());
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 58, 180, false);

        last_displayed_values[DOSE_CH] = ch_values[DOSE_CH].val;
    }
}

void _init_dose_rate_gui() {
    char string_buffer[10] = { 0 };
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm24b);

    Graphics_drawString(&g_context, "Dose rate", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);

    Graphics_setFont(&g_context, &g_sFontCm48b);

    sprintf(string_buffer, "%0.2e", ch_values[DOSE_RATE_CH].val);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 60, false);

    Graphics_setFont(&g_context, &g_sFontCm32b);
    Graphics_drawString(&g_context, "Gy/h", -1, 205, 60, false);

    draw_button(&to_menu_button);
    last_displayed_values[DOSE_RATE_CH] = ch_values[DOSE_RATE_CH].val;
}

void _update_dose_rate_gui() {
    if (last_displayed_values[DOSE_RATE_CH] != ch_values[DOSE_RATE_CH].val) {
        const Graphics_Rectangle hide_current = { .xMin = 4, .yMin = 45, .xMax = 204, .yMax = 110 };
        char string_buffer[10] = { 0 };

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_current);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_setFont(&g_context, &g_sFontCm48b);

        sprintf(string_buffer, "%0.2e", ch_values[DOSE_RATE_CH].val);
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 4, 60, false);

        last_displayed_values[DOSE_RATE_CH] = ch_values[DOSE_RATE_CH].val;
    }
}
