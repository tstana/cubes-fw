/**
 * @file      msp_endian.h
 * @author    John Wikman 
 * @copyright MIT License
 * @brief     Functions for endian conversion.
 *
 * @details
 * Declares function prototypes for converting a byte sequence from Big-Endian
 * into system defined integers.
 */

#ifndef MSP_ENDIAN_H
#define MSP_ENDIAN_H

void msp_to_bigendian32(unsigned char *dest, unsigned long number);
unsigned long msp_from_bigendian32(const unsigned char *src);

#endif /* MSP_ENDIAN_H */
