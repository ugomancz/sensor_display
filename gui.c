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


void update_display() {
    Graphics_setBackgroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    switch (current_context) {
    case FIND:
        Graphics_clearDisplay(&g_context);
        _update_display_find_lookup(comm_state == MESSAGE_RECEIVED);
        break;
    }
}

void _update_display_find_lookup(bool found) {
    int8_t string_buffer[30];
    Graphics_setFont(&g_context, &g_sFontCm24b);
    Graphics_drawString(&g_context, "Device lookup", -1, 4, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);
    sprintf((char *) &string_buffer,  "Scanning address: 0x%02x", device_address);
    Graphics_drawStringCentered(&g_context, string_buffer, -1, 160, 90, false);
    draw_button(&to_menu_button);
    if (found) {
        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_setFont(&g_context, &g_sFontCm22);
        Graphics_drawStringCentered(&g_context, "Device found! Use the device?", -1, 160, 130, false);
        draw_button(&find_accept_button);
        draw_button(&find_reject_button);
    }
}

void draw_button( button *b) {
    b->active = true;
    Graphics_setForegroundColor(&g_context, b->button_color);
    Graphics_fillRectangle(&g_context, &(b->coords));
    Graphics_setForegroundColor(&g_context, b->text_color);
    Graphics_setFont(&g_context, b->font);
    Graphics_drawStringCentered(&g_context, b->text, -1, (b->coords.xMax + b->coords.xMin) / 2, (b->coords.yMax + b->coords.yMin) / 2, false);
}
