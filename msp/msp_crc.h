/**
 * @file      msp_crc.h
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Functions for calculating CRC values.
 */

#ifndef MSP_CRC_H
#define MSP_CRC_H

unsigned long msp_crc32(const unsigned char *data, unsigned long len, unsigned long start_remainder);

#endif /* MSP_CRC_H */
