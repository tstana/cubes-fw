/*
 * main.c
 *
 *  Created on: 25 okt. 2018
 *  Author: Marcus Persson
 *  V1.0
 *  FPGA MIST I2C communication code
 */

#include <stdint.h>
#include "hvps/hvps_c11204-02.h"
#include "msp/msp_i2c_int.h"
#define SLAVE_ADDR 0x35
extern int *mem_addr = (int*) 0x60000000;

int main(void){
	init_i2c(SLAVE_ADDR);
	hvps_init(&mem_addr);
	while(1){}
}

/*
 * TODO: main
 * Find a way to format data to send to I2C
 *
 */
