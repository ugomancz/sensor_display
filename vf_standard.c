/*
 * mdg04.c
 *
 *  Created on: 4 Apr 2022
 *      Author: ondra
 */
#include <vf_standard.h>
#include <string.h>
#include <stdio.h>

/* Saves properly formatted HW_ID to buffer based on id */
void format_hw_id(char *buffer, uint32_t id) {
    char *p = buffer + strlen(buffer);
    uint32_t type = id >> 28;
    id &= 0x0FFFFFFF;
    switch (type) {
    case 0x00:
        sprintf(p++, "K");
        sprintf(p, "%04d-", id / 1000);
        p += 5;
        sprintf(p, "%02d-", (id / 100) % 100);
        p += 3;
        sprintf(p++, "C");
        sprintf(p, "%02d", id % 100);
        break;
    case 0x01:
        sprintf(p, "50-A-");
        p += 5;
        sprintf(p, "%07d", id);
        break;
    case 0x02:
        sprintf(p, "51-A-");
        p += 5;
        sprintf(p, "%07d", id);
        break;
    case 0x03:
        sprintf(p, "52-A-");
        p += 5;
        sprintf(p, "%07d", id);
        break;
    }
}

/* Saves properly formatted SW_ID to buffer based on id */
void format_sw_id(char *buffer, uint32_t id) {
    char *p = buffer + strlen(buffer);
    uint32_t type = id >> 28;
    id &= 0x0FFFFFFF;
    switch (type) {
    case 0x00:
        sprintf(p++, "K");
        sprintf(p, "%04d-", id / 1000);
        p += 5;
        sprintf(p, "%02d-", (id / 100) % 100);
        p += 3;
        sprintf(p++, "F");
        sprintf(p, "%02d", id % 100);
        break;
    case 0x04:
        sprintf(p, "53-A-");
        p += 5;
        sprintf(p, "%07d", id);
        break;
    case 0x05:
        sprintf(p, "54-A-");
        p += 5;
        sprintf(p, "%07d", id);
        break;
    case 0x08:
        sprintf(p++, "S");
        sprintf(p, "%04d-", id / 10000);
        p += 4;
        sprintf(p, "%02d-", (id / 100) % 100);
        p += 3;
        sprintf(p++, "F");
        sprintf(p, "%02d", id % 100);
        break;
    }
}
