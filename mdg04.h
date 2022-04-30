/*
 * mdg04.h
 *
 *  Created on: 14 Mar 2022
 *      Author: ondra
 */

#ifndef MDG04_H_
#define MDG04_H_

#include <stdint.h>

#define ID_REG_START_ADDR 0x0000
#define TEMP_REG_START_ADDR 0x0114
#define DOSE_REG_START_ADDR 0x010A
#define DOSE_RATE_REG_START_ADDR 0x0100

#define ID_REGS_COUNT 0x0024
#define CH_VAL_REGS_COUNT 0x000A

typedef struct {
    uint32_t hw_id;
    uint32_t sw_id;
    uint16_t sw_ver;
    uint16_t _res1;
    uint32_t ser_no;
    uint8_t pr_id[16];
    uint16_t _res2[8];
    uint8_t pr_name[16];
    uint32_t pl_id;
    uint16_t pl_ver;
    uint16_t _res3;
} dev_id;

typedef union {
    uint32_t time;
    uint32_t counter;
} ch_val_ts;

typedef struct {
    float val;
    uint32_t sta;
    uint32_t asi;
    ch_val_ts ts;
    uint16_t par_cnt;
    uint16_t _res;
} ch_val;

void switch_string_endianity(uint8_t *s);
void switch_float_endianity(float *f);
void format_hw_id(int8_t *buffer, uint32_t id);
void format_sw_id(int8_t *buffer, uint32_t id);

#endif /* MDG04_H_ */
