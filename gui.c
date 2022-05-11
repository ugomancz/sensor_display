/*
 * gui.c
 *
 * This file contains implementation of the GUI functionality.
 *
 * Author: Ondrej Kostik
 */
#include "gui.h"
#include "vf_standard.h"
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <stdio.h>
#include <string.h>

/* Structures used by the display driver */
extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
extern Graphics_Context g_context;

volatile _gui_context gui_context = DEVICE_LOOKUP_GUI;
typedef struct {
    float dose_rate;
    float dose;
    float temp;
} old_ch_values;

volatile uint8_t clr_screen = 1;
volatile bool update_gui = true;
static old_ch_values last_values = { 0 };

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
button lookup_accept_button = {
        .coords = { .xMin = 50, .yMin = 180, .xMax = 150, .yMax = 220 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm24b,
        .text = "Yes",
        .active = false
};

/* "No" button within DEVICE_LOOKUP_GUI context */
button lookup_reject_button = {
        .coords = { .xMin = 170, .yMin = 180, .xMax = 270, .yMax = 220 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm24b,
        .text = "No",
        .active = false
};

/* "Measurements" button within MENU_GUI context */
button to_values_button = {
        .coords = { .xMin = 10, .yMin = 180, .xMax = 155, .yMax = 230 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm20b,
        .text = "Measurements",
        .active = false
};

/* "Find Device" button within MENU_GUI context */
button to_lookup_button = {
        .coords = { .xMin = 165, .yMin = 180, .xMax = 310, .yMax = 230 },
        .button_color = GRAPHICS_COLOR_LIGHT_GRAY,
        .text_color = GRAPHICS_COLOR_BLACK,
        .font = &g_sFontCm18b,
        .text = "Device lookup",
        .active = false
};

/* Draws a button onto the display */
static void draw_button(button *b) {
    b->active = true;
    Graphics_setForegroundColor(&g_context, b->button_color);
    Graphics_fillRectangle(&g_context, &(b->coords));
    Graphics_setForegroundColor(&g_context, b->text_color);
    Graphics_setFont(&g_context, b->font);
    Graphics_drawStringCentered(&g_context, b->text, -1, (b->coords.xMax + b->coords.xMin) / 2,
            (b->coords.yMax + b->coords.yMin) / 2, false);
}

static void init_device_lookup_gui(sensor_info *lookup_sensor) {
    char string_buffer[10] = { 0 };
    Graphics_setFont(&g_context, &g_sFontCm24b);
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);

    Graphics_drawString(&g_context, "Device lookup", -1, 10, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);

    Graphics_setFont(&g_context, &g_sFontCm22b);
    Graphics_drawString(&g_context, "Scanning address:", -1, 10, 60, false);

    sprintf(string_buffer, "0x%02x", lookup_sensor->addr);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 200, 60, false);

    draw_button(&to_menu_button);
}

void update_found_device_lookup_gui(sensor_info *lookup_sensor) {
    char string_buffer[25] = { 0 };
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm22b);
    Graphics_drawStringCentered(&g_context, "Device found!", -1, 160, 98, false);

    Graphics_setFont(&g_context, &g_sFontCm20);

    sprintf(string_buffer, "Name: %s", lookup_sensor->id.pr_name);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 120, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "HW ID: ");
    format_hw_id(string_buffer, lookup_sensor->id.hw_id);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 140, false);

    draw_button(&lookup_accept_button);
    draw_button(&lookup_reject_button);
}

static void update_device_lookup_gui(sensor_info *lookup_sensor) {
    const Graphics_Rectangle hide_address = { .xMin = 200, .yMin = 55, .xMax = 285, .yMax = 80 };
    const Graphics_Rectangle hide_found = { .xMin = 0, .yMin = 85, .xMax = 319, .yMax = 239 };
    char string_buffer[10] = { 0 };

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&g_context, &hide_address);

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm22b);
    sprintf(string_buffer, "0x%02x", lookup_sensor->addr);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 200, 60, false);
    lookup_accept_button.active = false;
    lookup_reject_button.active = false;
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&g_context, &hide_found);
}

static void init_menu_gui(channels_data *ch_data, sensor_info *current_sensor) {
    char string_buffer[25] = { 0 };
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm24b);

    Graphics_drawString(&g_context, "Menu", -1, 10, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);

    Graphics_setFont(&g_context, &g_sFontCm20);

    sprintf(string_buffer, "Address: 0x%02x", current_sensor->addr);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 50, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "SN: %d", current_sensor->id.ser_no);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 70, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "HW ID: ");
    format_hw_id(string_buffer, current_sensor->id.hw_id);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 90, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "SW ID: ");
    format_sw_id(string_buffer, current_sensor->id.sw_id);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 110, false);

    memset(string_buffer, 0, 25);
    sprintf(string_buffer, "SW Ver.: %02d.%02d", (current_sensor->id.sw_ver >> 8), (uint8_t) current_sensor->id.sw_ver);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 130, false);

    memset(string_buffer, 0, 25);
    int chars_written = sprintf(string_buffer, "Temp: %0.2f ", ch_data->temp_val.val);
    sprintf(string_buffer + chars_written, UNITS[ch_data->temp_par.unit]);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 150, false);
    last_values.temp = ch_data->temp_val.val;

    draw_button(&to_values_button);
    draw_button(&to_lookup_button);
}

static void update_menu_gui(channels_data *ch_data, sensor_info *current_sensor) {
    if (last_values.temp != ch_data->temp_val.val) {
        const Graphics_Rectangle hide_temp = { .xMin = 65, .yMin = 150, .xMax = 175, .yMax = 175 };
        char string_buffer[20] = { 0 };

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_temp);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_setFont(&g_context, &g_sFontCm20);

        int chars_written = sprintf(string_buffer, "%0.2f ", ch_data->temp_val.val);
        sprintf(string_buffer + chars_written, UNITS[ch_data->temp_par.unit]);
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 75, 150, false);
        last_values.temp = ch_data->temp_val.val;
    }
}

static void init_values_gui(channels_data *ch_data) {
    char string_buffer[20] = { 0 };
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm24b);

    Graphics_drawString(&g_context, "Measurements", -1, 10, 9, false);
    Graphics_drawLineH(&g_context, 1, 320, 40);
    Graphics_drawString(&g_context, "Dose rate", -1, 10, 50, false);
    Graphics_drawLineH(&g_context, 1, 320, 140);
    Graphics_drawString(&g_context, "Dose", -1, 10, 150, false);

    Graphics_setFont(&g_context, &g_sFontCm48b);

    sprintf(string_buffer, "%0.2e", ch_data->dose_rate_val.val);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 80, false);

    memset(string_buffer, 0, 20);
    sprintf(string_buffer, "%0.2e", ch_data->dose_val.val);
    Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 180, false);

    Graphics_setFont(&g_context, &g_sFontCm32b);
    Graphics_drawString(&g_context, (int8_t*) UNITS[ch_data->dose_rate_par.unit], -1, 210, 90, false);
    Graphics_drawString(&g_context, (int8_t*) UNITS[ch_data->dose_par.unit], -1, 210, 190, false);

    draw_button(&to_menu_button);
    last_values.dose_rate = ch_data->dose_rate_val.val;
    last_values.dose = ch_data->dose_val.val;
}

static void update_values_gui(channels_data *ch_data) {
    char string_buffer[20] = { 0 };
    if (last_values.dose_rate != ch_data->dose_rate_val.val) {
        const Graphics_Rectangle hide_dose_rate = { .xMin = 10, .yMin = 75, .xMax = 210, .yMax = 130 };

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_dose_rate);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_setFont(&g_context, &g_sFontCm48b);

        sprintf(string_buffer, "%0.2e", ch_data->dose_rate_val.val);
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 80, false);

        last_values.dose_rate = ch_data->dose_rate_val.val;
    }
    if (last_values.dose = ch_data->dose_val.val) {
        const Graphics_Rectangle hide_dose = { .xMin = 10, .yMin = 175, .xMax = 210, .yMax = 230 };

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
        Graphics_fillRectangle(&g_context, &hide_dose);

        Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
        Graphics_setFont(&g_context, &g_sFontCm48b);

        memset(string_buffer, 0, 20);
        sprintf(string_buffer, "%0.2e", ch_data->dose_val.val);
        Graphics_drawString(&g_context, (int8_t*) string_buffer, -1, 10, 180, false);

        last_values.dose = ch_data->dose_val.val;
    }
}

static void init_error_gui() {
    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_RED);
    Graphics_setFont(&g_context, &g_sFontCm48b);

    Graphics_drawStringCentered(&g_context, (int8_t*) "Error!", -1, 160, 60, false);

    Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(&g_context, &g_sFontCm20b);

    Graphics_drawStringCentered(&g_context, (int8_t*) "Connection to the sensor lost.", -1, 160, 125, false);
    Graphics_drawStringCentered(&g_context, (int8_t*) "Reboot required.", -1, 160, 145, false);
}

/* Top level function which updates the screen contents based on the current GUI context */
void gui_update(volatile uint8_t *clr_screen, channels_data *ch_data, sensor_info *current_sensor,
        sensor_info *lookup_sensor) {
    if (*clr_screen > 0) {
        Graphics_clearDisplay(&g_context);
        to_values_button.active = false;
        to_lookup_button.active = false;
        lookup_accept_button.active = false;
        lookup_reject_button.active = false;
        to_menu_button.active = false;
    }
    switch (gui_context) {
    case DEVICE_LOOKUP_GUI:
        if (*clr_screen > 0) {
            init_device_lookup_gui(lookup_sensor);
        } else {
            update_device_lookup_gui(lookup_sensor);
        }
        break;
    case MENU_GUI:
        if (*clr_screen > 0) {
            init_menu_gui(ch_data, current_sensor);
        } else {
            update_menu_gui(ch_data, current_sensor);
        }
        break;
    case VALUES_GUI:
        if (*clr_screen > 0) {
            init_values_gui(ch_data);
        } else {
            update_values_gui(ch_data);
        }
        break;
    case ERROR_GUI:
        init_error_gui();
        break;
    }
    if (*clr_screen > 0) {
        --(*clr_screen);
    }
}
