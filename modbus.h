/*
 * modbus.h
 *
 *  Created on: 29 Apr 2022
 *      Author: ondra
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#include <stdint.h>

#define SUCCESS 0
#define INVALID_FN_CODE -1
#define OUT_OF_BOUNDS_READ -2
#define FN_CODE_RET_ERROR -3
#define CRC_ERROR -4
#define INCOMPLETE_DATA -5
#define INVALID_DEV_ADDR -6

/*
 * TODO: Ask about this
 *
 * Funkce jako je gen_mb_read_input_regs() by se mohla volat skrz wrapper, neco jako
 * mb_encode(dev_address, fn_code, fn_args[], void * output_dest). Ten by vevnitr mel switch a
 * podle fn_code by vytahl z fn_args konkretni argumenty a ty predal dedikovanym funkcim jako
 * je prave gen_mb_read_input_regs(). Pocty argumentu pro jednotlive funkce by byly definovane tady
 * v hlavickovem souboru.
 */

typedef enum { // TODO: Remove unused
    READ_HOLD_REGS = 0x03,
    READ_IN_REGS = 0x04,
    WRITE_SINGLE_REG = 0x06,
    WRITE_MULTIPLE_REGS = 0x10
} fn_code;

/*
 * This function takes device address, register read start address and number of registers to be read.
 * It then generates a ModBus-RTU ADU with "Read Input Registers" request and stores it in the output_dest.
 * After function is finished, *output_pos contains number of bytes written to output_dest.
 */
int gen_mb_read_input_regs(uint8_t device_addr, uint16_t reg_addr_start, uint16_t regs_count, uint8_t * output_dest, volatile uint8_t * output_pos);

/*
 * This function takes a ModBus-RTU "Read Input Registers" request response ADU, checks validity of
 * the response and parses the received data into output_dest.
 * In case of an error, an appropriate value is returned.
 */
int decode_mb_read_input_regs(uint8_t * data, uint8_t data_length, void * output_dest);

void switch_register_endianity(uint8_t *data, const unsigned int bytes);

#endif /* MODBUS_H_ */
