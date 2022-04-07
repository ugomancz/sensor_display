/*
 * gui.c
 *
 *  Created on: 17 Mar 2022
 *      Author: ondra
 */
#include "gui.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "communication.h"
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "application.h"
#include "mdg04.h"

volatile bool clr_screen = true;

button to_menu_button = {
    .coords = {.xMin = 248, .yMin = 2, .xMax = 318, .yMax = 37},
    .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
    .text_color = GRAPHICS_COLOR_BLACK,
    .font = &g_sFontCm22b,
    .text = "Menu",
    .active = false
};

button find_accept_button = {
    .coords = {.xMin = 50, .yMin = 180, .xMax = 150, .yMax = 220},
    .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
    .text_color = GRAPHICS_COLOR_BLACK,
    .font = &g_sFontCm24b,
    .text = "Yes",
    .active = false
};

button find_reject_button = {
    .coords = {.xMin = 170, .yMin = 180, .xMax = 270, .yMax = 220},
    .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
    .text_color = GRAPHICS_COLOR_BLACK,
    .font = &g_sFontCm24b,
    .text = "No",
    .active = false
};

button menu_dose_button = {
    .coords = {.xMin = 4, .yMin = 180, .xMax = 102, .yMax = 220},
    .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
    .text_color = GRAPHICS_COLOR_BLACK,
    .font = &g_sFontCm20b,
    .text = "Dose",
    .active = false
};

button menu_dose_rate_button = {
    .coords = {.xMin = 110, .yMin = 180, .xMax = 208, .yMax = 220},
    .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
    .text_color = GRAPHICS_COLOR_BLACK,
    .font = &g_sFontCm20b,
    .text = "Dose Rate",
    .active = false
};

button menu_find_button = {
    .coords = {.xMin = 216, .yMin = 180, .xMax = 316, .yMax = 220},
    .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
    .text_color = GRAPHICS_COLOR_BLACK,
    .font = &g_sFontCm20b,
    .text = "Find Dev.",
    .active = false
};

bool button_was_pressed(button *b, int32_t x, int32_t y) {
    return (b->coords.xMin <= x && b->coords.xMax >= x && b->coords.yMin <= y && b->coords.yMax >= y);
}

int32_t touchcallback(uint32_t message, int32_t x, int32_t y) {
    if (message == MSG_PTR_UP) {
        if (to_menu_button.active && button_was_pressed(&to_menu_button, x, y) || (find_accept_button.active && button_was_pressed(&find_accept_button, x, y))) {
            clr_screen = true;
            current_context = MENU;
            comm_state = IDLE;
            find_accept_button.active = false;
            find_reject_button.active = false;
            start_context_switch();
        }
        if (find_reject_button.active && button_was_pressed(&find_reject_button, x, y)) {
            comm_state = SEND_MESSAGE;
            ++device_address;
            reset_buffer();
            TimerLoadSet(TIMER1_BASE, TIMER_A, FIND_SEND_MSG_DELAY);
            TimerEnable(TIMER1_BASE, TIMER_A);
        }
        if (menu_find_button.active && button_was_pressed(&menu_find_button, x, y)) {
            clr_screen = true;
            current_context = FIND;
            comm_state = IDLE;
            device_address = 0x01;
            memset(&device_id, 0, sizeof(dev_id));
            menu_dose_button.active = false;
            menu_dose_rate_button.active = false;
            menu_find_button.active = false;
            start_context_switch();
        }
        if (menu_dose_button.active && button_was_pressed(&menu_dose_button, x, y)) {
            clr_screen = true;
            current_context = DOSE;
            comm_state = IDLE;
            menu_dose_button.active = false;
            menu_dose_rate_button.active = false;
            menu_find_button.active = false;
            start_context_switch();
        }
        if (menu_dose_rate_button.active && button_was_pressed(&menu_dose_rate_button, x, y)) {
            clr_screen = true;
            current_context = DOSE_RATE;
            comm_state = IDLE;
            menu_dose_button.active = false;
            menu_dose_rate_button.active = false;
            menu_find_button.active = false;
            start_context_switch();
        }
    }
    return 0;
}

void update_display() {
    Graphics_setBackgroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    if (clr_screen) {
        Graphics_clearDisplay(&g_context);
    }
    switch (current_context) {
    case FIND:
        if (clr_screen) {
            _init_display_find();
        } else {
            _update_display_find(comm_state == MESSAGE_RECEIVED);
        }
        break;
    case MENU:
        if (clr_screen) {
            _init_display_menu();
        } else {
            _update_display_menu();
        }
        break;
    case DOSE:
        if (clr_screen) {
            _init_display_dose();
        } else {
            _update_display_dose();
        }
    }
    clr_screen = false;
}

void _init_display_find() {
    Graphics_setFont(&g_context, &g_sFontCm24b);
    int8_t string_buffer[10];
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_drawString(&g_context, "Device lookup", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);
    Graphics_setFont(&g_context, &g_sFontCm22b);
    Graphics_drawString(&g_context, "Scanning address:", -1, 4, 60, false);
    sprintf((char *) &string_buffer, "0x%02x", device_address);
    Graphics_drawString(&g_context, string_buffer, -1, 200, 60, false);
}

void _update_display_find(bool found) {
    const Graphics_Rectangle hide_address = {.xMin = 200, .yMin = 55, .xMax = 280, .yMax = 80};
    const Graphics_Rectangle hide_found = {.xMin = 0, .yMin = 85, .xMax = 319, .yMax = 239};
    int8_t string_buffer[40];

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&g_context, &hide_address);

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    sprintf((char *) &string_buffer, "0x%02x", device_address);
    Graphics_drawString(&g_context, string_buffer, -1, 200, 60, false);
    if (found) {
        Graphics_setFont(&g_context, &g_sFontCm22b);
        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);

        sprintf((char *) &string_buffer,  "Device found!");
        Graphics_drawStringCentered(&g_context, string_buffer, -1, 160, 98, false);

        Graphics_setFont(&g_context, &g_sFontCm20);

        sprintf((char *) &string_buffer,  "Name: %s", device_id.pr_name);
        Graphics_drawString(&g_context, string_buffer, -1, 4, 120, false);

        sprintf((char *) string_buffer, "HW ID: ");
        format_hw_id(string_buffer, device_id.hw_id);
        Graphics_drawString(&g_context, string_buffer, -1, 4, 140, false);

        draw_button(&find_accept_button);
        draw_button(&find_reject_button);
    } else {
        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_found);
    }
}

void _init_display_menu() {
    to_menu_button.active = false;
    int8_t string_buffer[40];
    Graphics_setFont(&g_context, &g_sFontCm24b);
    Graphics_drawString(&g_context, "Menu", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm20);

    sprintf((char *) &string_buffer,  "Address: 0x%02x", device_address);
    Graphics_drawString(&g_context, string_buffer, -1, 4, 50, false);
    memset(string_buffer, 0, 40);

    sprintf((char *) &string_buffer,  "SN: %d", device_id.ser_no);
    Graphics_drawString(&g_context, string_buffer, -1, 4, 70, false);
    memset(string_buffer, 0, 40);

    sprintf((char *) string_buffer, "HW ID: ");
    format_hw_id(string_buffer, device_id.hw_id);
    Graphics_drawString(&g_context, string_buffer, -1, 4, 90, false);

    sprintf((char *) string_buffer, "SW ID: ");
    format_sw_id(string_buffer, device_id.sw_id);
    Graphics_drawString(&g_context, string_buffer, -1, 4, 110, false);

    sprintf((char *) &string_buffer,  "SW Ver.: %02d.%02d", (device_id.sw_ver >> 8), (char) device_id.sw_ver);
    Graphics_drawString(&g_context, string_buffer, -1, 4, 130, false);

    Graphics_drawString(&g_context, "Temp:", -1, 4, 150, false);

    draw_button(&menu_dose_button);
    draw_button(&menu_dose_rate_button);
    draw_button(&menu_find_button);
}

void _update_display_menu() {
    if (ch_value.val != 0.0f) {
        const Graphics_Rectangle hide_temp = {.xMin = 68, .yMin = 150, .xMax = 180, .yMax = 175};
        int8_t string_buffer[20] = {0};
        sprintf((char *) &string_buffer, "%0.2f deg. C", ch_value.val);
        Graphics_setFont(&g_context, &g_sFontCm20);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_temp);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_drawString(&g_context, string_buffer, -1, 70, 150, false);
    }
}

void _init_display_dose() {
    int8_t string_buffer[40] = {0};
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm24b);
    Graphics_drawString(&g_context, "Dose", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);

    Graphics_setFont(&g_context, &g_sFontCm48b);
    sprintf((char *) &string_buffer, "%0.3f", ch_value.val);
    Graphics_drawString(&g_context, string_buffer, -1, 4, 60, false);
    Graphics_setFont(&g_context, &g_sFontCm32b);
    Graphics_drawString(&g_context, "Gy", -1, 115, 70, false);

    Graphics_setFont(&g_context, &g_sFontCm22b);
    Graphics_drawString(&g_context, "Last minute extremes:", -1, 4, 120, false);

    Graphics_setFont(&g_context, &g_sFontCm20b);
    memset(&string_buffer, 0, 40);
    sprintf((char *) &string_buffer, "%0.3f", ch_value.val); // TODO: implement value history
    Graphics_drawString(&g_context, string_buffer, -1, 58, 155, false);
    Graphics_drawString(&g_context, "Min:", -1, 4, 155, false);
    Graphics_drawString(&g_context, "Gy", -1, 115, 155, false);

    memset(&string_buffer, 0, 40);
    sprintf((char *) &string_buffer, "%0.3f", ch_value.val); // TODO: implement value history
    Graphics_drawString(&g_context, string_buffer, -1, 58, 180, false);
    Graphics_drawString(&g_context, "Max:", -1, 4, 180, false);
    Graphics_drawString(&g_context, "Gy", -1, 115, 180, false);

    draw_button(&to_menu_button);
}

void _update_display_dose() {
    int8_t string_buffer[40] = {0};

    const Graphics_Rectangle hide_current = {.xMin = 4, .yMin = 45, .xMax = 112, .yMax = 100};
    const Graphics_Rectangle hide_min_max = {.xMin = 55, .yMin = 145, .xMax = 112, .yMax = 195};
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&g_context, &hide_current);
    Graphics_fillRectangle(&g_context, &hide_min_max);

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm48b);
    sprintf((char *) &string_buffer, "%0.3f", ch_value.val);
    Graphics_drawString(&g_context, string_buffer, -1, 4, 60, false);

    Graphics_setFont(&g_context, &g_sFontCm20b);
    memset(&string_buffer, 0, 40);
    sprintf((char *) &string_buffer, "%0.3f", ch_value.val); // TODO: implement value history
    Graphics_drawString(&g_context, string_buffer, -1, 58, 155, false);

    memset(&string_buffer, 0, 40);
    sprintf((char *) &string_buffer, "%0.3f", ch_value.val); // TODO: implement value history
    Graphics_drawString(&g_context, string_buffer, -1, 58, 180, false);
}

void draw_button(button *b) {
    b->active = true;
    Graphics_setForegroundColor(&g_context, b->button_color);
    Graphics_fillRectangle(&g_context, &(b->coords));
    Graphics_setForegroundColor(&g_context, b->text_color);
    Graphics_setFont(&g_context, b->font);
    Graphics_drawStringCentered(&g_context, b->text, -1, (b->coords.xMax + b->coords.xMin) / 2, (b->coords.yMax + b->coords.yMin) / 2, false);
}


