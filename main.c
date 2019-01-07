/*
 * main.c
 *
 *  Created on: 25 okt. 2018
 *  Author: Marcus Persson
 *  V1.0
 *  FPGA MIST I2C communication code
 */

#include <stdint.h>
#include "msp/i2c_comm_int.h"
#include "hvps/UART_comm_int.h"
#include "hvps/uart_comm_func.h"
#define SLAVE_ADDR 0x35




int main(void){
	init_i2c(SLAVE_ADDR);
	init_uart(NULL);
	while(1){}
}

/*
 * TODO: main
 * Make the ADC values go somewhere
 * Make a struct with ADC values and time
 * add histogram structure
 * Find a way to format data to send to I2C
 *
 */
