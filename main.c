/*
 * main.c
 *
 *  Created on: 25 okt. 2018
 *  Author: Marcus Persson
 *  V1.0
 *  FPGA MIST I2C communication code
 */

#include <stdint.h>
#include "Peripherals/FPGA_SYSINIT.h"
#include "Peripherals/I2C/i2c_comm_includes.h"
#include "Peripherals/UART/uart_comm_int.h"
#define SLAVE_ADDR 0x35

int main(void){
	init_i2c(SLAVE_ADDR);
	init_uart(NULL);
	while(1){

	}
}

