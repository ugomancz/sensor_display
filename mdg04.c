/*
 * mdg04.c
 *
 *  Created on: 4 Apr 2022
 *      Author: ondra
 */
#include "mdg04.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

void switch_string_endianity(uint8_t *s) {
    int l = (strlen((char *) s) + 1)/2;
    uint8_t temp = 0;
    for (int i = 0; i < l; i++) {
        temp = s[2*i+1];
        s[2*i+1] = s[2*i];
        s[2*i] = temp;
    }
}

void switch_float_endianity(float * const f) {
    uint8_t * const f_array = (uint8_t *) f;
    uint8_t temp = f_array[0];
    f_array[0] = f_array[1];
    f_array[1] = temp;
    temp = f_array[2];
    f_array[2] = f_array[3];
    f_array[3] = temp;
}

void format_hw_id(int8_t *buffer, uint32_t id) {
    char * p = (char *) buffer + strlen((char *) buffer);
    uint32_t type = id >> 28;
    id &= 0x0FFFFFFF;
    switch (type) {
    case 0x00:
        sprintf(p++, "K");
        sprintf(p, "%d-", id/1000);
        p += 5;
        sprintf(p, "%d-", (id/100)%100);
        p += 3;
        sprintf(p++, "C");
        sprintf(p, "%d", id%100);
        break;
    case 0x01:
        sprintf(p, "50-A-");
        p += 5;
        sprintf(p, "%d", id);
        break;
    case 0x02:
        sprintf(p, "51-A-");
        p += 5;
        sprintf(p, "%d", id);
        break;
    case 0x03:
        sprintf(p, "52-A-");
        p += 5;
        sprintf(p, "%d", id);
        break;
    }
}

void format_sw_id(int8_t *buffer, uint32_t id) {
    char * p = (char *) buffer + strlen((char *) buffer);
    uint32_t type = id >> 28;
    id &= 0x0FFFFFFF;
    switch (type) {
    case 0x00:
        sprintf(p++, "K");
        sprintf(p, "%d-", id/1000);
        p += 5;
        sprintf(p, "%d-", (id/100)%100);
        p += 3;
        sprintf(p++, "F");
        sprintf(p, "%d", id%100);
        break;
    case 0x04:
        sprintf(p, "53-A-");
        p += 5;
        sprintf(p, "%d", id);
        break;
    case 0x05:
        sprintf(p, "54-A-");
        p += 5;
        sprintf(p, "%d", id);
        break;
    case 0x08:
        sprintf(p++, "S");
        sprintf(p, "%d-", id/10000);
        p += 4;
        sprintf(p, "%d-", (id/100)%100);
        p += 3;
        sprintf(p++, "F");
        sprintf(p, "%d", id%100);
        break;
    }
}
