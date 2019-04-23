/*
 * I2C_comm_int.c
 *
 *  Created on: 26 okt. 2018
 *      Author: Marcus Persson
 */

#define SLAVE_SER_ADDR_0   	0x35
#define NVM_SEQFLAG_ADDR	0x6000F000

#include <stdint.h>
#include "msp_i2c.h"
#include "../firmware/drivers/mss_nvm/mss_nvm.h"
#include "../mem_mgmt/mem_mgmt.h"
uint8_t i2c_tx_buffer[550] = "";
uint8_t i2c_rx_buffer[550] = "";
uint32_t slave_buffer_size = 550;

mss_i2c_slave_handler_ret_t slave_write_handler(mss_i2c_instance_t * this_i2c,uint8_t * p_rx_data,uint16_t rx_size){
	code = msp_recv_callback(p_rx_data, rx_size);
	msp_send_callback((unsigned char *)i2c_tx_buffer, (unsigned long *)&slave_buffer_size);
	return MSS_I2C_REENABLE_SLAVE_RX;
}

void init_i2c(int SLAVE_SER_ADDR){
	MSS_I2C_init(&g_mss_i2c0, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_60); /* Clock can be set to any value as slave, not used */
	MSS_I2C_set_slave_tx_buffer(&g_mss_i2c0, i2c_tx_buffer, sizeof(i2c_tx_buffer)); /* Set buffers for RX and TX I2C */
	MSS_I2C_set_slave_rx_buffer(&g_mss_i2c0, i2c_rx_buffer, sizeof(i2c_rx_buffer));
	MSS_I2C_clear_gca(&g_mss_i2c0); /* Can not be adressed by a general call adress */
	MSS_I2C_register_write_handler(&g_mss_i2c0, slave_write_handler);
	MSS_I2C_enable_slave(&g_mss_i2c0);
}


int msp_save_seqflags(void){
	msp_seqflags_t seqflags = msp_exp_state_get_seqflags();
	nvm_status_t status;

	status = NVM_unlock(NVM_SEQFLAG_ADDR, sizeof(msp_seqflags_t));
	if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status)){
		status = NVM_write(NVM_SEQFLAG_ADDR, (uint8_t*)&seqflags, sizeof(msp_seqflags_t), NVM_LOCK_PAGE);
		if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status)){
			return 0;
		}
	}
	return -1;
}

void msp_read_seqflags(void){
	msp_seqflags_t seqflags;
	NVM_write(0xF000, 0, sizeof(msp_seqflags_t), NVM_DO_NOT_LOCK_PAGE);
	mem_read(NVM_SEQFLAG, &seqflags);
	msp_exp_state_initialize(seqflags);
}
