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
fn_code last_fn_used;
int16_t last_dev_addr = -1;

static void switch_register_endianity(uint8_t *data, const unsigned int bytes) {
    uint8_t temp;
    for (int i = 0; i < bytes; i += 2) {
        temp = data[i + 1];
        data[i + 1] = data[i];
        data[i] = temp;
    }
}

void mb_gen_read_input_regs(uint8_t device_addr, uint16_t reg_addr_start, uint16_t regs_count, uint8_t *output_dest,
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
}

int mb_decode_read_input_regs(uint8_t *data, uint8_t data_length, void *output_dest) {
    if (data[0] != last_dev_addr) {
        return INVALID_DEV_ADDR;
    }
    if (data[1] != last_fn_used) {
        if (data[1] >= 0x80) {
            ((uint8_t*) output_dest)[0] = data[2];
            return EXCEPTION_FN_CODE;
        }
        return INVALID_FN_CODE;
    }
    /*
     * Subtracting 5 because device address, function code, data length
     * byte and crc bytes don't count.
     */
    if (data[2] != data_length - 5) {
        return INCOMPLETE_DATA;
    }
    uint16_t message_crc = ((int16_t) data[data_length - 1] << 8) | data[data_length - 2];
    /* Subtracting 2 to strip the existing crc bytes. */
    if (message_crc != get_crc(data, data_length - 2)) {
        return CRC_ERROR;
    }
    /* Adding 3 to strip initial address, function code and data length bytes. */
    memcpy(output_dest, data + 3, data[2]);
    switch_register_endianity(output_dest, data[2]);

    return SUCCESS;
}

void mb_gen_write_single_reg(uint8_t device_addr, uint16_t reg_addr, uint16_t reg_value, uint8_t *output_dest,
        volatile uint8_t *output_pos) {
    output_dest[(*output_pos)++] = device_addr;
    output_dest[(*output_pos)++] = (uint8_t) WRITE_SINGLE_REG;
    output_dest[(*output_pos)++] = (uint8_t) (reg_addr >> BYTE_WIDTH);
    output_dest[(*output_pos)++] = (uint8_t) reg_addr;
    output_dest[(*output_pos)++] = (uint8_t) (reg_value >> BYTE_WIDTH);
    output_dest[(*output_pos)++] = (uint8_t) reg_value;

    uint16_t crc = get_crc(output_dest, *output_pos);

    output_dest[(*output_pos)++] = (uint8_t) crc;
    output_dest[(*output_pos)++] = (uint8_t) (crc >> BYTE_WIDTH);

    last_fn_used = WRITE_SINGLE_REG;
    last_dev_addr = device_addr;
}

int mb_decode_write_single_reg(uint8_t *data, uint8_t data_length, void *output_dest) {
    if (data[0] != last_dev_addr) {
        return INVALID_DEV_ADDR;
    }
    if (data[1] != last_fn_used) {
        if (data[1] >= 0x80) {
            ((uint8_t*) output_dest)[0] = data[2];
            return EXCEPTION_FN_CODE;
        }
        return INVALID_FN_CODE;
    }
    uint16_t message_crc = ((int16_t) data[data_length - 1] << 8) | data[data_length - 2];
    /* Subtracting 2 to strip the existing crc bytes. */
    if (message_crc != get_crc(data, data_length - 2)) {
        return CRC_ERROR;
    }
    /* Only copying the register address and the value written. */
    memcpy(output_dest, data + 2, data_length - 4);
    switch_register_endianity(output_dest, data_length - 4);

    return SUCCESS;
}
