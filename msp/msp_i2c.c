/*
 * I2C_comm_int.c
 *
 *  Created on: 26 okt. 2018
 *      Author: Marcus Persson
 */

#include <stdint.h>

#include "../firmware/drivers/mss_i2c/mss_i2c.h"
#include "../firmware/drivers/mss_nvm/mss_nvm.h"
#include "../mem_mgmt/mem_mgmt.h"

#include "msp_i2c.h"
#include "msp_exp.h"


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

/**
 * @brief Slave write handler for the I2C slave dedicated to MSP communication
 * @param this_i2c   Pointer to I2C interface (use g_mss_i2c0 or g_mss_i2c1)
 * @param p_rx_data  Receive data buffer
 * @param rx_size    Number of bytes that have been received in the receive data
 *                   buffer
 * @return MSS_I2C_REENABLE_SLAVE_RX  to indicate data buffer should be released
 *         MSS_I2C_PAUSE_SLAVE_RX     to indicate data buffer should _not_ be
 *                                    released
 */
static
mss_i2c_slave_handler_ret_t I2C1_SlaveWriteHandler(mss_i2c_instance_t * this_i2c,
                                                   uint8_t * p_rx_data,
                                                   uint16_t rx_size)
{
	msp_recv_callback(p_rx_data, rx_size);
	msp_send_callback((unsigned char *)i2c_tx_buffer, (unsigned long *)&slave_buffer_size);
	return MSS_I2C_REENABLE_SLAVE_RX;
}


/**
 * @brief  Initialize MSS_I2C1 for use with MSP
 *
 * @param slave_ser_addr  7-bit I2C address
 */
void msp_i2c_init(uint8_t slave_ser_addr)
{
	/* `ser_clock_speed` param not important, CUBES is an I2C slave... */
	MSS_I2C_init(&g_mss_i2c1, slave_ser_addr, MSS_I2C_PCLK_DIV_60);
	MSS_I2C_set_slave_tx_buffer(&g_mss_i2c1, i2c_tx_buffer, sizeof(i2c_tx_buffer));
	MSS_I2C_set_slave_rx_buffer(&g_mss_i2c1, i2c_rx_buffer, sizeof(i2c_rx_buffer));

	/* CUBES can not be adressed by a general call adress */
	MSS_I2C_clear_gca(&g_mss_i2c1);

	/* Register local write handler function and enable slave */
	MSS_I2C_register_write_handler(&g_mss_i2c1, I2C1_SlaveWriteHandler);
	MSS_I2C_enable_slave(&g_mss_i2c1);
	/* Set interrupt priority lower than UART's */
	NVIC_SetPriority(g_mss_i2c1.irqn, 1);
}
