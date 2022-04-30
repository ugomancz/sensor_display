/*
 * modbus.c
 *
 *  Created on: 29 Apr 2022
 *      Author: ondra
 */
#include "modbus.h"
#include "crc.h"
#include <string.h>

#define BYTE_WIDTH 8
volatile fn_code last_fn_used;
int16_t last_dev_addr = -1;

/*
 * This function takes device address, register read start address and number of registers to be read.
 * It then generates a ModBus-RTU ADU with "Read Input Registers" request and stores it in the output_dest.
 * After function is finished, *output_pos contains number of bytes written to output_dest.
 */
int gen_mb_read_input_regs(uint8_t device_addr, uint16_t reg_addr_start, uint16_t regs_count, uint8_t *output_dest,
        volatile uint8_t *output_pos) {
    output_dest[(*output_pos)++] = device_addr;
    output_dest[(*output_pos)++] = (uint8_t) READ_IN_REGS;
    output_dest[(*output_pos)++] = (uint8_t) (reg_addr_start >> BYTE_WIDTH);
    output_dest[(*output_pos)++] = (uint8_t) reg_addr_start;
    output_dest[(*output_pos)++] = (uint8_t) (regs_count >> BYTE_WIDTH);
    output_dest[(*output_pos)++] = (uint8_t) regs_count;

    uint16_t crc = get_crc(output_dest, *output_pos);

    output_dest[(*output_pos)++] = (uint8_t) crc;
    output_dest[(*output_pos)++] = (uint8_t) (crc >> BYTE_WIDTH);

    last_fn_used = READ_IN_REGS;
    last_dev_addr = device_addr;

    return SUCCESS;
}

/*
 * This function takes a ModBus-RTU "Read Input Registers" request response ADU, checks validity of
 * the response and parses the received data into output_dest.
 * As part of the parsing process, data is converted into little-endian.
 * In case of an error, an appropriate value is returned.
 */
int decode_mb_read_input_regs(uint8_t *data, uint8_t data_length, void *output_dest) {
    if (data[0] != last_dev_addr) {
        return INVALID_DEV_ADDR;
    }
    if (data[1] != last_fn_used) {
        return INVALID_FN_CODE;
    }
    /* Subtracting 5 because device address, function code, data length byte and crc bytes don't count */
    if (data[2] != data_length - 5) {
        return INCOMPLETE_DATA;
    }
    uint16_t message_crc = ((int16_t) data[data_length - 1] << 8) | data[data_length - 2];
    /* Subtracting 2 to strip the existing crc bytes */
    if (message_crc != get_crc(data, data_length - 2)) {
        return CRC_ERROR;
    }
    /* Adding 3 to strip initial address, function code and data length bytes */
    memcpy(output_dest, data + 3, data[2]);
    switch_register_endianity(output_dest, data[2]);

    return SUCCESS;
}

void switch_register_endianity(uint8_t *data, const unsigned int bytes) {
    uint8_t temp;
    for (int i = 0; i < bytes; i += 2) {
        temp = data[i + 1];
        data[i + 1] = data[i];
        data[i] = temp;
    }
}
