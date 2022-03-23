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

// Updates the display content based on the current context
void update_display();

/* Update/redraw display with specific screen. Used internally by update_display() */
void _update_display_find_lookup(bool found);
// TODO: Add other contexts

void draw_button(button *b);

#endif /* GUI_H_ */
