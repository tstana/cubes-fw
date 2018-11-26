/*
 * UART_comm_int.c
 *
 *  Created on: 6 nov. 2018
 *      Author: Marcus Persson
 */

#include "UART_comm_int.h"
#include "../I2C/i2c_comm_includes.h"



 /* UART handler for RX from HVPS */
void uart1_rx_handler(mss_uart_instance_t * this_uart){
	uint8_t rx_buff[16]="";
	uint32_t rx_size;
	uint8_t buffer[50]="";
	static unsigned int writing = 0;

	rx_size = MSS_UART_get_rx(this_uart, rx_buff, sizeof(rx_buff)); /* Get message from external and send it on to computer terminal */
	if(writing == 0)
		buffer[0] = '\0';
	if(rx_buff[0] != 0x0d){
		writing = 1;
		strncat((char*)buffer, rx_buff, rx_size);
	}
	else {
		writing = 0;
		MSS_UART_polled_tx_string(&g_mss_uart0, buffer);
	}
	memset(rx_buff, 0, sizeof(rx_buff)); /* Clear buffer */
}

/* UART handler for RX from external source */

void uart0_rx_handler(mss_uart_instance_t * this_uart){
	unsigned char rx_buff[10] ="";
	unsigned int rx_size;
	static unsigned int writing = 0;
	/* Get commands from terminal on connected computer and send them on to external*/
	rx_size = MSS_UART_get_rx(this_uart, rx_buff, sizeof(rx_buff));
	if(writing == 0)
		send_data_hk[0] = '\0';
	if(rx_buff[0] != 0x0d){
		writing = 1;
		strncat((char*)send_data_hk, rx_buff, rx_size);
	}
	else{
		writing = 0;
		MSS_UART_polled_tx_string(&g_mss_uart1, rx_buff);
	}
	/* Clear buffers */
	memset(rx_buff, 0, sizeof(rx_buff));
}
void initUART(void){
	/*
	 * Initialize and configure UART
	 * UART: 38400 BAUD, 8 bits, 1 stop bit, even parity
	 */

	MSS_UART_init(&g_mss_uart1, MSS_UART_38400_BAUD, MSS_UART_DATA_8_BITS | MSS_UART_EVEN_PARITY | MSS_UART_ONE_STOP_BIT);
	MSS_UART_init(&g_mss_uart0, MSS_UART_38400_BAUD, MSS_UART_DATA_8_BITS | MSS_UART_EVEN_PARITY | MSS_UART_ONE_STOP_BIT);
	MSS_UART_set_rx_handler(&g_mss_uart1, uart1_rx_handler, MSS_UART_FIFO_SINGLE_BYTE);
	MSS_UART_set_rx_handler(&g_mss_uart0, uart0_rx_handler, MSS_UART_FIFO_SINGLE_BYTE);
}
