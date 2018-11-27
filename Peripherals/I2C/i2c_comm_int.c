/*
 * I2C_comm_int.c
 *
 *  Created on: 26 okt. 2018
 *      Author: Marcus Persson
 */

//#define SLAVE_SER_ADDR_0   0x35

#include "i2c_comm_includes.h"
unsigned char txbuffer[500] = "";
unsigned char rxbuffer[500] = "";

void init_i2c(int SLAVE_SER_ADDR){
	MSS_I2C_init(&g_mss_i2c0, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_60); /* Clock can be set to any value as slave, not used */
	MSS_I2C_set_slave_tx_buffer(&g_mss_i2c0, txbuffer, sizeof(txbuffer)); /* Set buffers for RX and TX I2C */
	MSS_I2C_set_slave_rx_buffer(&g_mss_i2c0, rxbuffer, sizeof(rxbuffer));
	MSS_I2C_clear_gca(&g_mss_i2c0); /* Can not be adressed by a general call adress */
	MSS_I2C_enable_slave(&g_mss_i2c0);
}
