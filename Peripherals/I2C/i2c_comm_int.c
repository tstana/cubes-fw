/*
 * I2C_comm_int.c
 *
 *  Created on: 26 okt. 2018
 *      Author: Marcus Persson
 */

#define SLAVE_SER_ADDR_0   0x35

#include "i2c_comm_includes.h"
unsigned char i2c_tx_buffer[500] = "";
unsigned char i2c_rx_buffer[500] = "";
unsigned int slave_buffer_size = 500;

mss_i2c_slave_handler_ret_t slave_write_handler(mss_i2c_instance_t * this_i2c,uint8_t * p_rx_data,uint16_t rx_size){
	msp_recv_callback(p_rx_data, rx_size);
	msp_send_callback((unsigned char *)i2c_tx_buffer, (unsigned long *)&slave_buffer_size);
	return MSS_I2C_REENABLE_SLAVE_RX;
}

void init_i2c(int SLAVE_SER_ADDR){
	MSS_I2C_init(&g_mss_i2c0, SLAVE_SER_ADDR_0, MSS_I2C_PCLK_DIV_60); /* Clock can be set to any value as slave, not used */
	MSS_I2C_set_slave_tx_buffer(&g_mss_i2c0, i2c_tx_buffer, sizeof(i2c_tx_buffer)); /* Set buffers for RX and TX I2C */
	MSS_I2C_set_slave_rx_buffer(&g_mss_i2c0, i2c_rx_buffer, sizeof(i2c_rx_buffer));
	MSS_I2C_clear_gca(&g_mss_i2c0); /* Can not be adressed by a general call adress */
	MSS_I2C_register_write_handler(&g_mss_i2c0, slave_write_handler);
	MSS_I2C_enable_slave(&g_mss_i2c0);
}
