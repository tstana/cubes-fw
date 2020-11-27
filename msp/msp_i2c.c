/*
 * I2C_comm_int.c
 *
 *  Created on: 26 okt. 2018
 *      Author: Marcus Persson
 */

#define SLAVE_SER_ADDR_0   	0x35
#define NVM_SEQFLAG_ADDR	0x6000F000

#include <stdint.h>

#include "../firmware/drivers/mss_i2c/mss_i2c.h"
#include "../firmware/drivers/mss_nvm/mss_nvm.h"
#include "../mem_mgmt/mem_mgmt.h"

#include "msp_i2c.h"


/*
 * Define RX and TX buffers for I2C; their sizes are the max. data that is to
 * be sent via MSP. For the RX buffer, this is perhaps strictly not needed, but
 * better to avoid a HardFault in case the OBC tries to send CUBES more data
 * than it expects.
 *
 * Note: If the OBC tries to send it more than the MTU is another matter!
 */
static uint8_t i2c_tx_buffer[MSP_EXP_MAX_FRAME_SIZE];
static uint8_t i2c_rx_buffer[MSP_EXP_MAX_FRAME_SIZE];

static uint32_t slave_buffer_size = 0;

/*
 *
 */
static
mss_i2c_slave_handler_ret_t slave_write_handler(mss_i2c_instance_t * this_i2c,
                                                uint8_t * p_rx_data,
                                                uint16_t rx_size)
{
	msp_recv_callback(p_rx_data, rx_size);
	msp_send_callback((unsigned char *)i2c_tx_buffer, (unsigned long *)&slave_buffer_size);
	return MSS_I2C_REENABLE_SLAVE_RX;
}

/**
 * @brief Initialize MSS_I2C1 for use with MSP
 *
 * @param slave_ser_addr   I2C slave address of CUBES, set it to `MSP_EXP_ADDR`
 */
void msp_init_i2c(int slave_ser_addr)
{
	/* `ser_clock_speed` param not important, CUBES is an I2C slave... */
	MSS_I2C_init(&g_mss_i2c1, slave_ser_addr, MSS_I2C_PCLK_DIV_60);
	MSS_I2C_set_slave_tx_buffer(&g_mss_i2c1, i2c_tx_buffer, sizeof(i2c_tx_buffer));
	MSS_I2C_set_slave_rx_buffer(&g_mss_i2c1, i2c_rx_buffer, sizeof(i2c_rx_buffer));

	/* CUBES can not be adressed by a general call adress */
	MSS_I2C_clear_gca(&g_mss_i2c1);

	/* Register local write handler function and enable slave */
	MSS_I2C_register_write_handler(&g_mss_i2c1, slave_write_handler);
	MSS_I2C_enable_slave(&g_mss_i2c1);
	/* Set interrupt priority lower than UART's */
	NVIC_SetPriority(g_mss_i2c1.irqn, 1);
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
	mem_read(NVM_SEQFLAG, (uint32_t *)&seqflags);
	msp_exp_state_initialize(seqflags);
}
