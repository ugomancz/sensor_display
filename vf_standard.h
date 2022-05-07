/*
 * mdg04.h
 *
 *  Created on: 14 Mar 2022
 *      Author: ondra
 */

#ifndef VF_STANDARD_H_
#define VF_STANDARD_H_

#include <stdint.h>

#define ID_REG_START_ADDR 0x0000
#define DOSE_RATE_REG_START_ADDR 0x0100
#define DOSE_REG_START_ADDR 0x010A
#define TEMP_REG_START_ADDR 0x0114

#define DOSE_RATE_PAR_REG_START_ADDR 0x0200
#define DOSE_PAR_REG_START_ADDR 0x0220
#define TEMP_PAR_REG_START_ADDR 0x0240

#define ID_REGS_COUNT 0x0024
#define CH_VAL_REGS_COUNT 0x000A
#define CH_PAR_REGS_COUNT 0x0020

typedef union {
    uint32_t time;
    uint32_t counter;
} ch_val_ts;

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

typedef struct {
    float val;
    uint32_t sta;
    uint32_t asi;
    ch_val_ts ts;
    uint16_t par_cnt;
    uint16_t _res;
} ch_val;

typedef struct {
    float llmr;
    float ulmr;
    float tll1;
    float tll2;
    float tul1;
    float tul2;
    uint16_t pq;
    uint16_t _res1;
    uint16_t unit;
    uint16_t par_cnt;
    uint8_t ext[32];
} ch_par;

void format_hw_id(char *buffer, uint32_t id);
void format_sw_id(char *buffer, uint32_t id);

#endif /* VF_STANDARD_H_ */
