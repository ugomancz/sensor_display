/*
 * gui.h
 *
 *  Created on: 1 May 2022
 *      Author: ondra
 */

#ifndef GUI_H_
#define GUI_H_

#include "application.h"
#include "kentec.h"
#include "touch.h"
#include <ti/grlib/grlib.h>
#include <stdbool.h>
#include <stdint.h>

/* Represents a button */
typedef struct {
    Graphics_Rectangle coords;
    int32_t button_color;
    int32_t text_color;
    const Graphics_Font *font;
    int8_t *text;
    bool active; // currently displayed therefore should accept clicks
} button;

/* Represents the current GUI context (i.e. currently displayed screen) */
typedef enum {
    DEVICE_LOOKUP_GUI, MENU_GUI, VALUES_GUI, ERROR_GUI
} gui_context;

/* Structures used by the display driver */
extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
Graphics_Context g_context;

/* "Menu" button in top right corner */
extern button to_menu_button;

/* "Yes" button within DEVICE_LOOKUP_GUI context */
extern button lookup_accept_button;

/* "No" button within DEVICE_LOOKUP_GUI context */
extern button lookup_reject_button;

/* "Measurements" button within MENU_GUI context */
extern button to_values_button;

/* "Find Device" button within MENU_GUI context */
extern button to_lookup_button;

/* Tracks the current GUI context (i.e. currently displayed screen) */
extern volatile gui_context current_gui_context;

/* Draws a button onto the display */
void draw_button(button *b);

/* Top level function which updates the screen contents based on the current GUI context */
void gui_update(volatile uint8_t *clr_screen, channels_data *ch_data, sensor_info *current_sensor, sensor_info *lookup_sensor);

/* Draws information about found device and buttons for user interaction (use/don't use choice) */
void update_found_device_lookup_gui();

/* Used internally by gui_update() */
void _init_device_lookup_gui();
void _update_device_lookup_gui();
void _init_menu_gui();
void _update_menu_gui();
void _init_values_gui();
void _update_values_gui();
void _init_error_gui();

#endif /* GUI_H_ */
