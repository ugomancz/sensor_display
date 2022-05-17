/*
 * gui.h
 *
 * This file contains declarations of variables and functions used
 * to control the GUI.
 *
 * Author: Ondrej Kostik
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
    SENSOR_LOOKUP_GUI, MENU_GUI, MEASUREMENTS_GUI, ERROR_GUI
} _gui_context;

/* Structures used by the display driver */
extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
Graphics_Context g_context;

/* "Menu" button in top right corner */
extern button to_menu_button;

/* "Yes" button within SENSOR_LOOKUP_GUI context */
extern button lookup_accept_button;

/* "No" button within SENSOR_LOOKUP_GUI context */
extern button lookup_reject_button;

/* "Measurements" button within MENU_GUI context */
extern button to_measurements_button;

/* "Find Sensor" button within MENU_GUI context */
extern button to_lookup_button;

/* Tracks the current GUI context (i.e. currently displayed screen) */
extern volatile _gui_context gui_context;

/* Tracks whether the whole screen should be cleared before drawing new GUI elements.
 * Typically happens when GUI context is switched.
 */
extern volatile uint8_t clr_screen;

/* Flag set to true if a GUI update is required */
extern volatile bool update_gui;

/* Top level function which updates the screen contents based on the current GUI context */
void gui_update(volatile uint8_t *clr_screen, channels_data *ch_data, sensor_info *current_sensor,
        sensor_info *lookup_sensor);

/* Draws information about found sensor and buttons for user interaction (use/don't use choice) */
void update_found_sensor_lookup_gui();

#endif /* GUI_H_ */
