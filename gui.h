/*
 * gui.h
 *
 *  Created on: 17 Mar 2022
 *      Author: ondra
 */

#ifndef GUI_H_
#define GUI_H_

#include <stdint.h>
#include <ti/grlib/grlib.h>
#include "globals.h"
#include "kentec.h"
#include "touch.h"

extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
Graphics_Context g_context;
extern volatile bool clr_screen;


/* This struct is used to represent buttons */
typedef struct {
    Graphics_Rectangle coords;
    int32_t button_color;
    int32_t text_color;
    const Graphics_Font *font;
    int8_t *text;
    bool active; // currently displayed -> should accept clicks
} button;

// Always present "Menu" button
extern button to_menu_button;

// FIND context optional buttons
extern button find_accept_button;
extern button find_reject_button;

int32_t touchcallback(uint32_t message, int32_t x, int32_t y);

// Updates the display content based on the current context
void update_display();

/* Update/redraw display with specific screen. Used internally by update_display() */
void _init_display_find();
void _update_display_find(bool found);
void _init_display_menu();
void _update_display_menu();
// TODO: Add other contexts

void draw_button(button *b);

#endif /* GUI_H_ */
