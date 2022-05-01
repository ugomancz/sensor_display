/*
 * mdg04.c
 *
 *  Created on: 4 Apr 2022
 *      Author: ondra
 */
#include "mdg04.h"
#include <string.h>
#include <stdio.h>

/* Saves properly formatted HW_ID to buffer based on id */
void format_hw_id(int8_t *buffer, uint32_t id) {
    char *p = (char*) buffer + strlen((char*) buffer);
    uint32_t type = id >> 28;
    id &= 0x0FFFFFFF;
    switch (type) {
    case 0x00:
        sprintf(p++, "K");
        sprintf(p, "%d-", id / 1000);
        p += 5;
        sprintf(p, "%d-", (id / 100) % 100);
        p += 3;
        sprintf(p++, "C");
        sprintf(p, "%d", id % 100);
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

/* Saves properly formatted SW_ID to buffer based on id */
void format_sw_id(int8_t *buffer, uint32_t id) {
    char *p = (char*) buffer + strlen((char*) buffer);
    uint32_t type = id >> 28;
    id &= 0x0FFFFFFF;
    switch (type) {
    case 0x00:
        sprintf(p++, "K");
        sprintf(p, "%d-", id / 1000);
        p += 5;
        sprintf(p, "%d-", (id / 100) % 100);
        p += 3;
        sprintf(p++, "F");
        sprintf(p, "%d", id % 100);
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
        sprintf(p, "%d-", id / 10000);
        p += 4;
        sprintf(p, "%d-", (id / 100) % 100);
        p += 3;
        sprintf(p++, "F");
        sprintf(p, "%d", id % 100);
        break;
    }
}
