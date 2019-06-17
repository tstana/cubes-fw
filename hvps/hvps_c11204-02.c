/*
 * hvps_c11204-02.c
 *
 *  Created on: 10 jan. 2019
 *      Author: Marcus Persson
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../firmware/drivers/mss_uart/mss_uart.h"
#include "../firmware/drivers/mss_timer/mss_timer.h"
#include "../firmware/drivers/mss_nvm/mss_nvm.h"
#include "hvps_c11204-02.h"
#include "../msp/msp_exp_handler.h"

#include "../mem_mgmt/mem_mgmt.h"



static uint8_t chkstr[2];
static uint8_t send[40];
static uint32_t *memadr;

static void getarray(uint8_t *array, uint8_t cmd[28]){
	const uint8_t stx = 0x02;
	const uint8_t etx = 0x03;
	const uint8_t CR = 0x0D;
	uint16_t chksm=0x00;
	/* Memmove is used with offset for the adress because strcat did not give the proper format when sending it on to the HVPS */
	int cmdlen = strlen(cmd);
	memmove(array, &stx, 1);
	memmove(array+1, cmd, cmdlen);
	memmove(array+1+cmdlen, &etx, 1);
	for(int i = 0; array[i-1]!=0x03; i++){
		chksm+=array[i];
	}
	chksm = (chksm & 0xFF); /* Mask so only lower 2 bytes get sent */
	sprintf(chkstr, "%02X", chksm);
	memmove(array+2+cmdlen, chkstr, 2);
	memmove(array+4+cmdlen, &CR, 1);
	memset(cmd, '\0', sizeof(cmd));
}





static int voltage_check(uint8_t cmd[28]){
	uint8_t data[4] = "";
	double val = 0;
	/* Check for which command that came to decide on array location */
	if((cmd[0]=='H' && cmd[1]=='S' && cmd[2]=='T')) {
		for(int i=0; i<4; i++){
			data[i] = cmd[i+19];
		}
	}
	else if(cmd[0]=='H' && cmd[1]=='B' && cmd[2]=='V'){
		for(int i=0; i<4; i++){
			data[i] = cmd[i+3];
		}
	}
	/* Convert to long and check value for limit of 55 */
	val=strtol((char*)data, NULL, 16);
	val=val*(1.812/pow(10, 3));
	if(val > 55)
		return -1;
	else
		return 0;
}

static void start_hvps(void){
	uint8_t nvm[28] = "HST";
	mem_read(NVM_HVPS, &nvm[3]);
	if(voltage_check(nvm)==-1)
		return;
	getarray(send, nvm); /*get required string from function */
	MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
	memset(send, '\0', sizeof(send));
}

int hvps_set_voltage(uint8_t* command){
	uint8_t HST[30]="HST"; /* Standard input, ~44.5V, no temp correction */
	for (int j=0; j<24; j++){
		HST[j]=memadr[j];
	}
	for (int i=0; i<4; i++){ /* Move voltage into temperature correction factor command */
		HST[19+i]=command[i];
	}
	if(voltage_check(HST) == -1)
		return -1;
	getarray(send, HST); /* Format string to UART and send it on */
	MSS_UART_polled_tx(&g_mss_uart0, send,strlen(send));
	memset(send, '\0', sizeof(send));
	return 0;
}

void hvps_turn_on(void){
	uint8_t HON[] = "HON";
	getarray(send, HON);
	MSS_TIM64_start();
	MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
	memset(send, '\0', sizeof(send));
}

void hvps_turn_off(void){
	uint8_t HOF[] = "HOF";
	getarray(send, HOF);
	MSS_TIM64_stop();
	MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
	memset(send, '\0', sizeof(send));
}

void hvps_save_voltage(void){
	uint8_t HRT[]="HRT";
	getarray(send, HRT);
	MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
	memset(send, '\0', sizeof(send));
}

void hvps_get_voltage(void){
	uint8_t HGV[]="HGV";
	getarray(send, HGV);
	MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
	memset(send, '\0', sizeof(send));
}

void hvps_get_current(void){
	uint8_t HGC[]="HGC";
	getarray(send, HGC);
	MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
	memset(send, '\0', sizeof(send));
}
void hvps_get_temp(void){
	uint8_t HGT[]="HGT";
	getarray(send, HGT);
	MSS_UART_polled_tx(&g_mss_uart0, send, strlen(send));
	memset(send, '\0', sizeof(send));
}

 /* UART handler for RX from HVPS */
/* TODO: Add some sort of process to check for what command got returned, if status, write to memory, if return from sent command, acknowledge or ignore */
static void uart0_rx_handler(mss_uart_instance_t * this_uart){
	uint8_t rx_buff[30]="";
	uint32_t rx_size;

	static uint8_t output[30]="";

	rx_size = MSS_UART_get_rx(this_uart, rx_buff, sizeof(rx_buff)); /* Get message from HVPS and send it on to computer terminal */
	if(rx_buff[rx_size-1] != 0x0d){
		strncat((char*)output, (char *)rx_buff, rx_size);
	}
	else {
		strncat((char*)output, (char*)rx_buff, rx_size);
		if(output[1]=='h' && output[2]=='g')
			msp_add_hk(&output[4], 4);
		else if(output[1]=='h' && output[2]=='r' && output[3]=='t'){
			mem_nvm_write(NVM_HVPS, &output[4]);
		}
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
	hvps_get_voltage();
	MSS_TIM64_clear_irq(); /*interrupt bit needs to be cleared after every call */
}


void hvps_init(uint32_t memory){
	/*
	 * Initialize and configure UART and timer
	 * Timer: periodic mode, loads value in load_immediate
	 * UART: 38400 BAUD, 8 bits, 1 stop bit, even parity
	 */

	memadr= (uint32_t*)memory;
	MSS_UART_init(&g_mss_uart0, MSS_UART_38400_BAUD, MSS_UART_DATA_8_BITS | MSS_UART_EVEN_PARITY | MSS_UART_ONE_STOP_BIT);
	MSS_UART_set_rx_handler(&g_mss_uart0, uart0_rx_handler, MSS_UART_FIFO_FOUR_BYTES);
	start_hvps();
	MSS_TIM64_init(MSS_TIMER_PERIODIC_MODE);
	MSS_TIM64_load_immediate(0x00000000, 0x00FFFFFF);
	MSS_TIM64_enable_irq();
	//MSS_TIM64_start();
}
