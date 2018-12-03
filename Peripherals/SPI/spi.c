/*
 * gpio.c
 *
 *  Created on: 19 nov. 2018
 *      Author: Marcus Persson
 */

#include "spi.h"


void init_spi(void){
	MSS_SPI_init(&g_mss_spi0);
	MSS_SPI_init(&g_mss_spi1);
    MSS_SPI_configure_master_mode(&g_mss_spi0,MSS_SPI_SLAVE_0,MSS_SPI_MODE3,256u,MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE); /* TODO: Set div so that 50kHz<freq<80MHz */
    MSS_SPI_configure_master_mode(&g_mss_spi1,MSS_SPI_SLAVE_1,MSS_SPI_MODE3,256u,MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE);
}



