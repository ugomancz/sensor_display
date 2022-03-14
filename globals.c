/*
 * globals.c
 *
 *  Created on: 14 Mar 2022
 *      Author: ondra
 */

#include "globals.h"
#include <stdlib.h>


/*
 * Main buffer for storing incoming messages.
 */
uint8_t *buffer;
uint16_t buffer_position;
