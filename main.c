/*
 * main.c
 *
 *  Created on: 25 okt. 2018
 *  Author: Marcus Persson
 *  V1.0
 *  FPGA MIST I2C communication code
 */

#include <stdint.h>
#include "Peripherals/UART/UART_comm_int.h"
#include "Peripherals/FPGA_SYSINIT.h"
#include "Peripherals/I2C/i2c_comm_includes.h"
#define SLAVE_ADDR 0x11

int main(void){
	initI2C(SLAVE_ADDR);
	initUART();
	while(1){

	}
}

