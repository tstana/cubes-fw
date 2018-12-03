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
#include "Peripherals/GPIO/gpio.h"
#include "Peripherals/SPI/spi.h"
#include "Peripherals/RTC/rtc.h"
#define SLAVE_ADDR 0x35

int main(void){
	init_i2c(SLAVE_ADDR);
	init_uart(NULL);
	init_spi();
	init_gpio();
	init_rtc();
	while(1){
	}
}

/*
 * TODO: main
 * Make the ADC values go somewhere
 * Make a struct with ADC values and time
 * add histogram structure
 * Find a way to format data to send to I2C
 *
 */
