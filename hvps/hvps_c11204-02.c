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
#include "../msp/msp_i2c.h"

#include "../mem_mgmt/mem_mgmt.h"



static uint8_t hvps_cmd_array[40];

static uint16_t hvps_status;
static uint8_t hvps_hk[12];
static uint16_t hvps_sent = 0;
static uint16_t hvps_ack = 0;
static uint16_t hvps_failed = 0;



static void prep_hvps_cmd_array(char *cmd)
{
	const uint8_t STX = 0x02;
	const uint8_t ETX = 0x03;
	const uint8_t CR = 0x0D;

	uint16_t chksm=0x00;
	char chkstr[3];

	/* Start with fresh command array */
	memset(hvps_cmd_array, '\0', sizeof(hvps_cmd_array));

	/*
	 * Memmove is used with offset for the adress because strcat did not give
	 * the proper format when sending it on to the HVPS
	 */
	int cmdlen = strlen((char *)cmd);
	memmove(hvps_cmd_array, &STX, 1);
	memmove(hvps_cmd_array+1, cmd, cmdlen);
	memmove(hvps_cmd_array+1+cmdlen, &ETX, 1);
	for(int i = 0; hvps_cmd_array[i-1] != 0x03; i++){
		chksm+=hvps_cmd_array[i];
	}
	chksm = (chksm & 0xFF); /* Mask so only lower 2 bytes get sent */
	sprintf(chkstr, "%02X", chksm);
	memmove(hvps_cmd_array+2+cmdlen, chkstr, 2);
	memmove(hvps_cmd_array+4+cmdlen, &CR, 1);
}

/* Prepares HVPS configuration for NVM saving and saves writes it to memory. */
static void hvps_to_mem(uint8_t data[24])
{
	uint8_t temp[4] ="";
	uint16_t temp2[6];
	for(int i=0; i<=24; i=i+4)
	{
		/* For every 4 bytes, copy over to temp variable and convert into integer*/
		memcpy(temp, data+i, 4);
		temp2[i/4]= strtol((char *)temp, NULL, 16);
	}
	mem_nvm_write(NVM_HVPS, (uint8_t *)temp2);
}

/* Reads the HVPS settings from memory and converts them into usable string in the
 * array provided by parameters.
 */
static void hvps_from_mem(char *data)
{
	/* Start by reading HVPS settings from NVM */
	uint32_t hvps_settings[3];
	mem_read(NVM_HVPS, hvps_settings);

	/* Convert these into ASCII; see memory layout diagram for details */
	uint16_t dtp1, dtp2;
	uint16_t dt1, dt2;
	uint16_t v, t;
	dtp1 = hvps_settings[0] & 0xFFFF;
	dtp2 = (hvps_settings[0] & 0xFFFF0000) >> 16;
	dt1 = hvps_settings[1] & 0xFFFF;
	dt2 = (hvps_settings[1] & 0xFFFF0000) >> 16;
	v = hvps_settings[2] & 0xFFFF;
	t = (hvps_settings[2] & 0xFFFF0000) >> 16;

	/* Compose command string, converting int16's into ASCII*/
	sprintf(&data[3], "%04X%04X%04X%04X%04X%04X", dtp1, dtp2, dt1, dt2, v, t);
}



static int voltage_check(char *cmd)
{
	char data[4] = "";
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
	val=strtol(data, NULL, 16);
	val=val*(1.812/pow(10, 3));
	if(val > 55)
		return -1;
	else
		return 0;
}

static void start_hvps(void)
{	/* Compose command string, converting int16's into ASCII*/
	char HST[28] = "HST000000000000000000000000";
	hvps_from_mem(HST);

	if(voltage_check(HST)==-1)
		return;

	/* Prepend STX and append checksum and CR, then hvps_cmd_array*/
	prep_hvps_cmd_array(HST);
	MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array, strlen((char *)hvps_cmd_array));
	hvps_sent++;
}

int hvps_set_temp_corr_factor(uint8_t* command)
{
	char HST[28]="HST";

	/* Convert input into ASCII; see memory layout diagram for details */
	uint16_t dtp1, dtp2;
	uint16_t dt1, dt2;
	uint16_t v, t;
	dtp1 = (command[0]<<8) | command[1];
	dtp2 = (command[2]<<8) | command[3];
	dt1 = (command[4]<<8) | command[5];
	dt2 = (command[6]<<8) | command[7];
	v = (command[8]<<8) | command[9];
	t = (command[10]<<8) | command[11];

	/* Compose command string, converting int16's into ASCII*/
	sprintf(&HST[3], "%04X%04X%04X%04X%04X%04X", dtp1, dtp2, dt1, dt2, v, t);
	if(voltage_check(HST) == -1)
		return -1;
	prep_hvps_cmd_array(HST); /* Format string to UART and hvps_cmd_array it on */
	MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array,strlen((char *)hvps_cmd_array));
	hvps_sent++;
	return 0;
}

int hvps_set_temporary_voltage(uint16_t v)
{
	/* Prep command string and parameter */
	char cmd[8] = "HBV";
	sprintf(&cmd[3], "%04X", v);

	/* Give up early if voltage is too high */
	if(voltage_check(cmd) == -1)
		return -1;

	/* Format string to UART and hvps_cmd_array it on */
	prep_hvps_cmd_array(cmd);
	MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array, strlen((char *)hvps_cmd_array));
	hvps_sent++;

	return 0;
}

void hvps_send_cmd(char *cmd)
{
	prep_hvps_cmd_array(cmd);
	MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array, strlen((char *)hvps_cmd_array));
	hvps_sent++;
}

uint8_t hvps_is_on(void)
{
	return (uint8_t)(hvps_status & 0x0001);
}

uint16_t hvps_get_latest_temp(void)
{
	uint8_t values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for(int i=0; i<4; i++){
		values[i] = hvps_hk[8+i];
	}
	uint16_t temp = strtol((char*)values, NULL, 16);
	return temp;
}
uint16_t hvps_get_latest_volt(void)
{
	uint8_t values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for(int i=0; i<4; i++){
		values[i] = hvps_hk[i];
	}
	uint16_t volt = strtol((char*)values, NULL, 16);
	return volt;
}
uint16_t hvps_get_latest_curr(void)
{
	uint8_t values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for(int i=0; i<4; i++){
		values[i] = hvps_hk[4+i];
	}
	uint16_t curr = strtol((char*)values, NULL, 16);
	return curr;
}
uint16_t hvps_get_com_val(uint8_t val)
{
	switch(val){
	case 1:
		return hvps_sent;
		break;
	case 2:
		return hvps_ack;
		break;
	case 3:
		return hvps_failed;
		break;
	default:
		return 0;
	}
}

/* UART handler for RX from HVPS */

static void uart0_rx_handler(mss_uart_instance_t * this_uart)
{
	// TODO: Make rx_buff 51-byte long, to account for max HVPS reply length.
	static uint8_t rx_buff[16]="";
	static size_t rx_size;

	rx_size += MSS_UART_get_rx(this_uart, rx_buff + rx_size, sizeof(rx_buff));
	if(rx_buff[rx_size-1] == 0x0d)
	{
		/* Increment command counters based on reply */
		if ((rx_buff[1] == hvps_cmd_array[1] + 0x20) &&
				(rx_buff[2] == hvps_cmd_array[2] + 0x20) &&
				(rx_buff[3] == hvps_cmd_array[3] + 0x20))
			hvps_ack++;
		else if(rx_buff[1] == 'h' && rx_buff[2] == 'x' && rx_buff[3] == 'x')
			hvps_failed++;

		/* Copy to HK buffer */
		if(rx_buff[1]=='h' && rx_buff[2]=='g' && rx_buff[3]=='v'){
			memcpy(&hvps_hk[0], &rx_buff[4], 4);
		}
		else if(rx_buff[1]=='h' && rx_buff[2]=='g' && rx_buff[3]=='c'){
			memcpy(&hvps_hk[4], &rx_buff[4], 4);
		}
		else if(rx_buff[1]=='h' && rx_buff[2]=='g' && rx_buff[3]=='t'){
			memcpy(&hvps_hk[8], &rx_buff[4], 4);
		}
		else if(rx_buff[1]=='h' && rx_buff[2]=='r' && rx_buff[3]=='t'){
			hvps_to_mem(&rx_buff[4]);
		}
		else if (rx_buff[1] == 'h' && rx_buff[2] == 'g' && rx_buff[3] == 's')
		{
			hvps_status = (uint16_t) strtol((char *)&rx_buff[4], NULL, 16);
		}

		/* Clear RX buffer and prep for next round of RX... */
		memset(rx_buff, '\0', sizeof(rx_buff));
		rx_size = 0;
	}
}

/**
 * @brief Timer interrupt for sending commands to the HVPS
 *        Each second, a separate command is sent to the HVPS.
 *
 */
void Timer1_IRQHandler(void)
{
	static uint8_t current_run = 0;
	char cmd[4] = "HG-";

	switch (current_run)
	{
	case 0:
		cmd[2] = 'S';
		hvps_send_cmd(cmd);
		break;
	case 1:
		cmd[2] = 'V';
		hvps_send_cmd(cmd);
		break;
	case 2:
		cmd[2] = 'C';
		hvps_send_cmd(cmd);
		break;
	case 3:
		cmd[2] = 'T';
		hvps_send_cmd(cmd);
		break;
	case 4:
		msp_add_hk(hvps_hk, 12, 16);
		break;
	default:
		break;
	}

	/* Increment current run counter */
	current_run = (current_run + 1) % 5;

	/* Interrupt bit needs to be cleared after every call */
	MSS_TIM1_clear_irq();
}



/**
 * Initialize and configure UART and timer
 * Timer: periodic mode, loads value in load_immediate
 * UART: 38400 BAUD, 8 bits, 1 stop bit, even parity
 */
void hvps_init(void)
{
	MSS_UART_init(&g_mss_uart0, MSS_UART_38400_BAUD, MSS_UART_DATA_8_BITS |
			MSS_UART_EVEN_PARITY | MSS_UART_ONE_STOP_BIT);
	NVIC_SetPriority(UART0_IRQn, 1);
	MSS_UART_set_rx_handler(&g_mss_uart0, uart0_rx_handler, MSS_UART_FIFO_FOUR_BYTES);
	start_hvps();

	/* Set a 1-second timeout on the timer (multiply with 100MHz clock freq.)*/
	unsigned long long settimer  = 1 * 100000000;
	unsigned long timer1 = settimer & 0xFFFFFFFF;
	MSS_TIM2_init(MSS_TIMER_ONE_SHOT_MODE);
	MSS_TIM2_load_immediate(timer1);
	MSS_TIM2_enable_irq();
	MSS_TIM1_init(MSS_TIMER_PERIODIC_MODE);
	MSS_TIM1_load_immediate(timer1);
	MSS_TIM1_enable_irq();
	MSS_TIM1_start();
}



