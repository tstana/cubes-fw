/*
 * gpio.c
 *
 *  Created on: 3 dec. 2018
 *      Author: Marcus Persson
 */

#include "gpio.h"
#include "../SPI/spi.h"

uint8_t ADC_incoming_data[10]="";
volatile uint16_t adc_incoming_flag = 0;
void init_gpio(void){
	MSS_GPIO_init();
	MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE); /* TODO: add more rows and change GPIO pin depending on what is connected */

	MSS_GPIO_enable_irq(MSS_GPIO_1);
}


void GPIO1_IRQHandler(void){
	MSS_SPI_set_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );
	MSS_SPI_transfer_block(&g_mss_spi0, 0, 0, ADC_incoming_data, 2);
	MSS_SPI_clear_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );
	adc_incoming_flag=1;
	MSS_GPIO_clear_irq(MSS_GPIO_1);
}
