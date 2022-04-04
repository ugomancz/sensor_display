/*
 * gui.c
 *
 *  Created on: 17 Mar 2022
 *      Author: ondra
 */
#include "gui.h"
#include <stdbool.h>
#include <stdio.h>
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

bool button_was_pressed(button *b, int32_t x, int32_t y) {
    return (b->coords.xMin <= x && b->coords.xMax >= x && b->coords.yMin <= y && b->coords.yMax >= y);
}

int32_t touchcallback(uint32_t message, int32_t x, int32_t y) {
    if (message == MSG_PTR_UP) {
        if (button_was_pressed(&to_menu_button, x, y) || (find_accept_button.active && button_was_pressed(&find_accept_button, x, y))) {
            clr_screen = true;
            current_context = MENU;
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
    }
    return 0;
}

void update_display() {
    Graphics_setBackgroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    if (clr_screen) {
        Graphics_clearDisplay(&g_context);
        clr_screen = false;
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
            //_update_display_menu();
        }
    }
}

void _init_display_find() {
    Graphics_setFont(&g_context, &g_sFontCm24b);
    int8_t string_buffer[10];
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_drawString(&g_context, "Device lookup", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);
    Graphics_drawString(&g_context, "Scanning address:", -1, 4, 60, false);
    sprintf((char *) &string_buffer, "0x%02x", device_address);
    Graphics_drawString(&g_context, string_buffer, -1, 220, 60, false);
    draw_button(&to_menu_button);
}

void _update_display_find(bool found) {
    const Graphics_Rectangle hide_address = {.xMin = 218, .yMin = 55, .xMax = 300, .yMax = 80};
    const Graphics_Rectangle hide_found = {.xMin = 0, .yMin = 85, .xMax = 319, .yMax = 239};

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&g_context, &hide_address);
    _init_display_find();
    if (found) {
        int8_t string_buffer[40];
        Graphics_setFont(&g_context, &g_sFontCm22b);
        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);

        sprintf((char *) &string_buffer,  "Device found!");
        Graphics_drawStringCentered(&g_context, string_buffer, -1, 160, 98, false);

        Graphics_setFont(&g_context, &g_sFontCm20);

        sprintf((char *) &string_buffer,  "Name: %s", switch_endianity(device_id.pr_name));
        Graphics_drawString(&g_context, string_buffer, -1, 4, 120, false);

        sprintf((char *) &string_buffer,  "HW ID: %d", device_id.ser_no);
        Graphics_drawString(&g_context, string_buffer, -1, 4, 140, false);

        draw_button(&find_accept_button);
        draw_button(&find_reject_button);
    } else {
        Graphics_fillRectangle(&g_context, &hide_found);
    }
}

void _init_display_menu() {
    int8_t string_buffer[30];
    Graphics_setFont(&g_context, &g_sFontCm24b);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_drawString(&g_context, "Menu", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);
    sprintf((char *) &string_buffer,  "Scanning address: 0x%02x", device_address);
    //Graphics_drawStringCentered(&g_context, string_buffer, -1, 160, 90, false);
    draw_button(&to_menu_button);
}

void draw_button( button *b) {
    b->active = true;
    Graphics_setForegroundColor(&g_context, b->button_color);
    Graphics_fillRectangle(&g_context, &(b->coords));
    Graphics_setForegroundColor(&g_context, b->text_color);
    Graphics_setFont(&g_context, b->font);
    Graphics_drawStringCentered(&g_context, b->text, -1, (b->coords.xMax + b->coords.xMin) / 2, (b->coords.yMax + b->coords.yMin) / 2, false);
}


