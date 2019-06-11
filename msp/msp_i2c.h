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
#include "msp_exp.h"

extern unsigned int has_send;
extern unsigned int has_recv;
extern unsigned int has_syscommand;

/* **********************************************
 * void initI2C(void)
 * Function for initializing I2C parameters and interrupts
 * Input:  slave_adress, on format 0x35
 * Output: None
 ********************************************** */

void init_i2c(int slave_adress);

/* **************************************************
 * int msp_save_seqflags
 * Function for saving MSP sequenceflags to NVM memory at system power-off.
 * Input: none
 * Return: -1 if any failure, 0 if passed
 **************************************************** */
int msp_save_seqflags(void);


/* ****************************************************
 * int msp_save_seqflags
 * Function for reading MSP sequenceflags from NVM memory at system power-on.
 * Input: none
 * Return: none
 *************************************************** */
void msp_read_seqflags(void);
#endif /* I2C_COMM_INT_H_ */
