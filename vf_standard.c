/*
 * vf_standard.c
 *
 * This file contains implementation of the UNITS "map" and functions
 * declared by the header file.
 *
 * Author: Ondrej Kostik
 */
#include <vf_standard.h>
#include <string.h>
#include <stdio.h>

const char UNITS[50][12] = { { "NaN" }, { "cps" }, { "cpm" }, { "Bq" }, { "Bq/cm2" },
        { "Bq/m3" }, { "Gy/h" }, { "Sv/h" }, { "R/h" }, { "dpm" }, { "deg. C" }, { "K" },
        { "Pa" }, { "m3" }, { "cps/cm2" }, { "m3/s" }, { "n/(cm2/s)" }, { "%" }, { "Gy/s" },
        { "Sv/s" }, { "m3/h" }, { "deg. F" }, { "Ci" }, { "Psi" }, { "cm3" }, { "ft3/min" },
        { "rem/h" }, { "rad/h" }, { "Ci/cm3" }, { "Ci/cm2" }, { "l/min" }, { "n/(cm2)" },
        { "1/s" }, { "1/(cm2/s)" }, { "1/(cm2/min)" }, { "ppm" }, { "Gy" }, { "Sv" },
        { "kg" }, { "V" }, { "A" }, { "m/s" }, { "s" }, { "kg/h" }, { "pps" }, { "pps/cm2" },
        { "min" }, { "h" }, { "m" }, { "W" } };

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
