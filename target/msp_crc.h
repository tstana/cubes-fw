/*
 * msp_crc.h
 * Author: John Wikman
 */

#ifndef MSP_CRC_H
#define MSP_CRC_H

/* Computes CRC32 */
unsigned long crc32(const unsigned char *data, unsigned long len, unsigned long start_remainder);

#endif
