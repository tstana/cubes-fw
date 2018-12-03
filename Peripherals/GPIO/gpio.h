/*
 * gpio.h
 *
 *  Created on: 3 dec. 2018
 *      Author: Marcus Persson
 */

#ifndef PERIPHERALS_GPIO_GPIO_H_
#define PERIPHERALS_GPIO_GPIO_H_

#include "../../firmware/drivers/mss_gpio/mss_gpio.h"

extern volatile uint16_t adc_incoming_flag;
extern uint8_t adc_incoming_data[10];

void init_gpio(void);

#endif /* PERIPHERALS_GPIO_GPIO_H_ */
