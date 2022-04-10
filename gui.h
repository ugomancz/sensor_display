/*
 * gui.h
 *
 *  Created on: 17 Mar 2022
 *      Author: ondra
 */
#ifndef GUI_H_
#define GUI_H_

#include "kentec.h"
#include "touch.h"
#include <ti/grlib/grlib.h>
#include <stdbool.h>
#include <stdint.h>

extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
Graphics_Context g_context;
extern volatile bool clr_screen;

/* This structure is used to represent buttons */
typedef struct {
    Graphics_Rectangle coords;
    int32_t button_color;
    int32_t text_color;
    const Graphics_Font *font;
    int8_t *text;
    bool active; // currently displayed therefore should accept clicks
} button;

/* Touch screen interrupt handler */
int32_t touch_callback(uint32_t message, int32_t x, int32_t y);

/* Top level function to update the display content based on the current context */
void display_update();

/* Update/redraw display with specific screen. Used internally by display_update() */
void _init_display_find();
void _update_display_find(bool found);
void _init_display_menu();
void _update_display_menu();
void _init_display_dose();
void _update_display_dose();
void _init_display_dose_rate();
void _update_display_dose_rate();

/* Draws a button on the display and sets b.active to true */
void draw_button(button *b);

#endif /* GUI_H_ */
