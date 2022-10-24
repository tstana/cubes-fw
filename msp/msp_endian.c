/**
 * @file      msp_endian.c
 * @author    John Wikman 
 * @copyright MIT License
 * @brief     Defines functions for endian conversion.
 *
 * @details
 * Provides functions for converting Big Endian byte sequences into integers
 * and vice versa.
 */

#include "msp_endian.h"

/**
 * @brief Converts a number into big-endian.
 * @param dest Pointer to where the converted number will be stored. The place
 *             that is pointed to must be able to store at least 4 bytes.
 * @param number The number to be converted into big-endian.
 *
 * Converts the entered number into a sequence of 4 bytes that follow
 * big-endian byte-order. The bytes will be stored at the location pointed to
 * by the dest argument.
 */
void msp_to_bigendian32(unsigned char *dest, unsigned long number)
{
	dest[0] = (unsigned char) (number >> 24) & 0xff;
	dest[1] = (unsigned char) (number >> 16) & 0xff;
	dest[2] = (unsigned char) (number >> 8)  & 0xff;
	dest[3] = (unsigned char) (number >> 0)  & 0xff;
}

/**
 * @brief Converts a sequence of bytes from big-endian.
 * @param src Pointer to a sequence of 4 bytes that will be converted from
 *            big-endian.
 * @return The converted number.
 *
 * Converts a sequence of 4 bytes from big-endian into a 4 byte unsigned
 * integer with the native platform endianness.
 */
unsigned long msp_from_bigendian32(const unsigned char *src)
{
	unsigned long res = 0;
	unsigned char i;
	for (i = 0; i < 4; i++) {
		res = res << 8;
		res += *(src + i);
	}

	return res;
}
