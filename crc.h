/*
 * Taken from VF Standard Q-7P1-06 document by VF, a.s.
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

extern const uint16_t crc_table[];

uint16_t get_crc(uint8_t *nData, uint16_t len);

#endif /* CRC_H_ */
