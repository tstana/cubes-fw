/*
 * I2C_comm_int.h
 *
 *  Created on: 26 okt. 2018
 *      Author: Marcus Persson
 */

#ifndef I2C_COMM_INT_H_
#define I2C_COMM_INT_H_

/***********************************************
 * void initI2C(void)
 * Function for initializing I2C parameters and interrupts
 * Input:  slave_adress, on format 0x35
 * Output: None
 ***********************************************/
void initI2C(int slave_adress);

#endif /* I2C_COMM_INT_H_ */
