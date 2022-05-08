/*
 * mdg04.h
 *
 * Author: Ondrej Kostik
 */
#ifndef VF_STANDARD_H_
#define VF_STANDARD_H_

#include <stdint.h>
#include <time.h>

/* Register address of the start of the DEV_ID structure in the sensors memory. */
#define ID_REG_START_ADDR 0x0000

/* Register addresses of the CH_VAL structures of the fast channels. */
#define DOSE_RATE_REG_START_ADDR 0x0100
#define DOSE_REG_START_ADDR 0x010A
#define TEMP_REG_START_ADDR 0x0114

/* Register addresses of the CH_PAR structures of the fast channels. */
#define DOSE_RATE_PAR_REG_START_ADDR 0x0200
#define DOSE_PAR_REG_START_ADDR 0x0220
#define TEMP_PAR_REG_START_ADDR 0x0240

/* Length of DEV_ID, CH_VAL and CH_PAR structures in number of registers. */
#define ID_REGS_COUNT 0x0024
#define CH_VAL_REGS_COUNT 0x000A
#define CH_PAR_REGS_COUNT 0x0020

/* Structure to store the sensor's identification. */
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

/* Structure to store the channel value information. */
typedef struct {
    float val;
    uint32_t sta;
    uint32_t asi;
    time_t ts;
    uint16_t par_cnt;
    uint16_t _res;
} ch_val;

/* Structure to store the channel parameters information. */
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

/*
 * Array of strings of different units which are defined by VF standard as
 * a number. It's used similarly to a map/dictionary data structure as known
 * from other languages.
 */
extern const char UNITS[50][12];

/*
 * Saves properly formatted HW_ID to buffer based on id. If buffer already
 * contains some characters, the formatted HW_ID is placed after them.
 */
void format_hw_id(char *buffer, uint32_t id);

/*
 * Saves properly formatted SW_ID to buffer based on id. If buffer already
 * contains some characters, the formatted SW_ID is placed after them.
 */
void format_sw_id(char *buffer, uint32_t id);

#endif /* VF_STANDARD_H_ */
