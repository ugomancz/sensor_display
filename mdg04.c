/*
 * mdg04.c
 *
 *  Created on: 4 Apr 2022
 *      Author: ondra
 */
#include "mdg04.h"
#include <string.h>

uint8_t * switch_endianity(uint8_t *s) {
    int l = (strlen((char *) s) + 1)/2;
    uint8_t temp = 0;
    for (int i = 0; i < l; i++) {
        temp = s[2*i+1];
        s[2*i+1] = s[2*i];
        s[2*i] = temp;
    }
    return s;
}
