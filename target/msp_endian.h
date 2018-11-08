/*
 * msp_endian.h
 * Author: John Wikman
 *
 * Defines function prototypes for converting a byte sequence from Big-Endian
 * into system defined integers.
 */

#ifndef MSP_ENDIAN_H
#define MSP_ENDIAN_H

void to_bigendian32(unsigned char *dest, unsigned long number);
unsigned long from_bigendian32(const unsigned char *src);

#endif
