/*
 * gui.h
 *
 *  Created on: 17 Mar 2022
 *      Author: ondra
 */

#ifndef GUI_H_
#define GUI_H_

#include <ti/grlib/grlib.h>
#include "globals.h"
#include "kentec.h"
#include "touch.h"

extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;
Graphics_Context sContext;

// Updates the display content based on the given context
void update_display(context_state context);

/* Update/redraw display with specific screen. Used internally by update_display() */
void _update_display_find();
// TODO: Add other contexts

#endif /* GUI_H_ */
