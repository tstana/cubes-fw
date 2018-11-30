/*
 * gpio.c
 *
 *  Created on: 19 nov. 2018
 *      Author: Marcus Persson
 */

#include "SPI.h"
extern volatile int ADC_incoming_data[500];

void SPI_slave_handler();

void initSPI(void){
	MSS_SPI_init(&g_mss_spi0);
	MSS_SPI_init(&g_mss_spi1);
    MSS_SPI_configure_master_mode(&g_mss_spi0,MSS_SPI_SLAVE_0,MSS_SPI_MODE2,256u,MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE);
}

void SPI_slave_handler(uint32_t slave_buffer){
	static int top=0;
	ADC_incoming_data[top++] = slave_buffer;
	SPI_flag=1;
	if(top==500)
		top=0;

}

