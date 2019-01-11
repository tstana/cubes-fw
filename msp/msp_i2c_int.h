/*
 * I2C_comm_int.h
 *
 *  Created on: 26 okt. 2018
 *      Author: Marcus Persson
 */

#ifndef I2C_COMM_INT_H_
#define I2C_COMM_INT_H_

#include <string.h>
#include <stdio.h>
#include "../firmware/CMSIS/system_m2sxxx.h"
#include "../firmware/drivers/mss_i2c/mss_i2c.h"
#include "../msp/msp_exp.h"

extern unsigned int has_send;
extern unsigned int has_recv;
extern unsigned char time_data[100];
extern volatile unsigned char send_data_hk[];

/***********************************************
 * void initI2C(void)
 * Function for initializing I2C parameters and interrupts
 * Input:  slave_adress, on format 0x35
 * Output: None
 ***********************************************/
void init_i2c(int slave_adress);

#endif /* I2C_COMM_INT_H_ */
