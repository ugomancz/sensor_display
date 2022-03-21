/*
 * gui.c
 *
 *  Created on: 17 Mar 2022
 *      Author: ondra
 */
#include "gui.h"
#include <stdbool.h>
#include <stdio.h>


void update_display(context_state context) {
    switch (context) {
    case FIND:
        Graphics_clearDisplay(&g_context);
        _update_display_find_lookup();
        Graphics_flushBuffer(&g_context);
        break;
    }
}

void _update_display_find_lookup() {
    int8_t string_buffer[30];
    Graphics_setBackgroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm22);
    Graphics_drawStringCentered(&g_context, "Device lookup", -1, 160, 15, false);
    Graphics_drawLineH(&g_context, 1, 320, 30);
    sprintf((char *) &string_buffer,  "Scanning address: 0x%02x", device_id);
    Graphics_drawStringCentered(&g_context, string_buffer, -1, 160, 100, false);
}
