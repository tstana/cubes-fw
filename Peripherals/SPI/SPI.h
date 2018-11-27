/*
 * gpio.h
 *
 *  Created on: 19 nov. 2018
 *      Author: Marcus Persson
 */

#include "../../firmware/drivers/mss_spi/mss_spi.h"

#ifndef PERIPHERALS_SPI_SPI_H_
#define PERIPHERALS_SPI_SPI_H_
extern volatile int SPI_flag;

void initSPI(void);
#endif /* PERIPHERALS_SPI_SPI_H_ */
