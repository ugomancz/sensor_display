/*
 * gui.h
 *
 *  Created on: 1 May 2022
 *      Author: ondra
 */

#ifndef GUI_H_
#define GUI_H_

#include "communication.h"
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

/* Tracks whether the whole screen should be cleared before drawing new GUI elements.
 * Typically happens when GUI context is switched.
 */
extern volatile bool clr_screen;

/* Flag set to true if a GUI update is required */
extern volatile bool update_gui;

/* Tracks the current GUI context (i.e. currently displayed screen) */
extern volatile gui_context current_gui_context;

/* Holds last displayed values to prevent unnecessary redrawing and screen flickering */
extern float last_displayed_values[3];

/* Touch screen interrupt handler */
int32_t touch_callback(uint32_t message, int32_t x, int32_t y);

/* Draws a button onto the display */
void draw_button(button *b);

/* Top level function which updates the screen contents based on the current GUI context */
void gui_update();

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
