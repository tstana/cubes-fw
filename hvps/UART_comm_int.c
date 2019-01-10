/*
 * UART_comm_int.c
 *
 *  Created on: 6 nov. 2018
 *      Author: Marcus Persson
 */

#include <string.h>
#include <stdio.h>
#include "../cubes-fw-libero/drivers/mss_uart/mss_uart.h"
#include "../cubes-fw-libero/drivers/mss_timer/mss_timer.h"
#include "uart_comm_int.h"
#include "uart_comm_func.h"


char *memadr;
extern volatile unsigned char send_data_hk[];



 /* UART handler for RX from HVPS */
/* TODO: Add some sort of process to check for what command got returned, if status, write to memory, if return from sent command, acknowledge or ignore */
void uart0_rx_handler(mss_uart_instance_t * this_uart){
	uint8_t rx_buff[30]="";
	uint32_t rx_size;
	static unsigned char output[30]="";

	rx_size = MSS_UART_get_rx(this_uart, rx_buff, sizeof(rx_buff)); /* Get message from HVPS and send it on to computer terminal */
	if(rx_buff[rx_size-1] != 0x0d){
		strncat(output, rx_buff, rx_size);
	}
	else {
		strncat(output, rx_buff, rx_size);
		strcpy(send_data_hk,output);
		memset(output, '\0', sizeof(output));
	}



	//processData(rx_buff); /* Process data for certain commands */
	memset(rx_buff, '\0', sizeof(rx_buff)); /* Clear buffer */
}


/* Timer interrupt for sending commands to the HVPS
 * Timing set up in timer1 init
 *
 */
void Timer1_IRQHandler(void){
	static uint16_t cntr=0;
	uint8_t send[32];
	uint8_t command[4] = "";
	if(cntr==0){
		/* Command for getting Voltage output */
		strcpy(command, "HGV");
		getarray(send, command);
		MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
		memset(send, '\0', sizeof(send));
	}
	if(cntr==1){
		/* Command for getting current output */
		strcpy(command, "HGC");
		getarray(send, command);
		MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
		memset(send, '\0', sizeof(send));
	}
	if(cntr==2){
		/* Command for getting Temperature output */
		strcpy(command, "HGT");
		getarray(send, command);
		MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
		memset(send, '\0', sizeof(send));
	}
	cntr=(cntr+1)%3;
	MSS_TIM64_clear_irq(); /*interrupt bit needs to be cleared after every call */
}


void init_uart(char* memory){
	/*
	 * Initialize and configure UART and timer
	 * Timer: periodic mode, loads value in load_immediate
	 * UART: 38400 BAUD, 8 bits, 1 stop bit, even parity
	 */

	memadr=memory;
	MSS_TIM64_init(MSS_TIMER_PERIODIC_MODE);
	MSS_TIM64_load_immediate(0x00000000, 0x0FFFFFFF);
	MSS_UART_init(&g_mss_uart0, MSS_UART_38400_BAUD, MSS_UART_DATA_8_BITS | MSS_UART_EVEN_PARITY | MSS_UART_ONE_STOP_BIT);
	MSS_UART_set_rx_handler(&g_mss_uart0, uart0_rx_handler, MSS_UART_FIFO_FOUR_BYTES);
	MSS_TIM64_enable_irq();
	MSS_TIM64_start();
}
