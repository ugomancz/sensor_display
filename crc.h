/*
 * Taken from VF Standard Q-7P1-06 document by VF, a.s.
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

extern const uint16_t crc_table[];

uint16_t get_crc(uint8_t *nData, uint16_t len);
uint16_t get_frame_crc(uint8_t slave_addr, uint8_t fn_code, uint8_t *nData, uint16_t len);
uint16_t to_modbus_compatible(uint16_t original_crc);

#endif /* CRC_H_ */
