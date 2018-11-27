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
	MSS_SPI_set_frame_rx_handler(&g_mss_spi0, SPI_slave_handler);
	int SPI_flag = 0;
}

void SPI_slave_handler(uint32_t slave_buffer){
	static int top=0;
	ADC_incoming_data[top++] = slave_buffer;
	SPI_flag=1;
	if(top==500)
		top=0;

}

