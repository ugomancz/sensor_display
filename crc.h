/*
 * crc.h
 *
 * CRC implementation taken from VF Standard Q-7P1-06 document by VF, a.s.
 *
 */
#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

/* Calculates the CRC of the given data using a CRC table. */
uint16_t get_crc(const uint8_t *data, uint16_t len);

#endif /* CRC_H_ */
