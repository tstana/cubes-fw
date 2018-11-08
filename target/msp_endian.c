/*
 * msp_endian.c
 * Author: John Wikman
 *
 * Provides functions for converting Big Endian byte sequences into integers
 * and vice versa.
 */

#include "msp_endian.h"

/*
 * Converts the entered number into a sequence of bytes that follows a Big
 * Endian byte order.
 */
void to_bigendian32(unsigned char *dest, unsigned long number)
{
  dest[0] = (unsigned char) (number >> 24) & 0xff;
  dest[1] = (unsigned char) (number >> 16) & 0xff;
  dest[2] = (unsigned char) (number >> 8)  & 0xff;
  dest[3] = (unsigned char) (number >> 0)  & 0xff;
}

/*
 * Converts the 4 bytes stored at the pointer into a 32-bit unsigned integer
 * based on a Big Endian byte order.
 */
unsigned long from_bigendian32(const unsigned char *src)
{
  unsigned long res = 0;
  unsigned char i;
  for (i = 0; i < 4; i++) {
    res = res << 8;
    res += *(src + i);
  }

  return res;
}
