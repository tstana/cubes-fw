/*
 * Source file for communicating to Hamamatsu C11204-02 MPPC bias module
 *
 *  Created on: 5 Nov. 2020
 *
 * Copyright © 2020 Theodor Stana (based on old code by Marcus Persson)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../firmware/drivers/mss_uart/mss_uart.h"
#include "../firmware/drivers/mss_nvm/mss_nvm.h"
#include "hvps_c11204-02.h"
#include "../msp/msp_i2c.h"

#include "../mem_mgmt/mem_mgmt.h"


/* Local Function Prototypes */
static void UART0_RXHandler(mss_uart_instance_t * this_uart);


/* Local Variables */
static uint8_t hvps_cmd_array[40];

static uint16_t hvps_status;
static uint8_t hvps_hk[12];
static uint16_t cmds_sent = 0;
static uint16_t cmds_acked = 0;
static uint16_t cmds_failed = 0;
static uint8_t wait = 0;




#include <../firmware/drivers/mss_timer/mss_timer.h>



/**
 * @brief Initialize and configure UART
 *
 * Configure MMUART_0 for communication to the C11204-02 HVPS module:
 *   - 38400 baud
 *   - 8 bits
 *   - 1 stop bit
 *   - even parity
 */
void hvps_init(void)
{
	/* Init UART for communicating to HVPS module */
	MSS_UART_init(&g_mss_uart0, MSS_UART_38400_BAUD, MSS_UART_DATA_8_BITS |
			MSS_UART_EVEN_PARITY | MSS_UART_ONE_STOP_BIT);
	NVIC_SetPriority(UART0_IRQn, 1);
	MSS_UART_set_rx_handler(&g_mss_uart0, UART0_RXHandler,
			MSS_UART_FIFO_FOUR_BYTES);



   unsigned long long settimer  = 1 * 100000000;
   unsigned long timer1 = settimer & 0xFFFFFFFF;
   MSS_TIM1_init(MSS_TIMER_PERIODIC_MODE);
   MSS_TIM1_load_immediate(timer1);
   MSS_TIM1_enable_irq();
   NVIC_SetPriority(Timer1_IRQn, 2);
   MSS_TIM1_start();



	/*
	 * -------------------------------------
	 * Write default HVPS setting from NVM
	 * -------------------------------------
	 */
//	/* Compose command string, converting int16's into ASCII */
//	// TODO: Figure out what default string stands for...
//	char HST[28] = "HST0000000004090409757DB7D7";
//
//	/* Start by reading HVPS settings from NVM */
//	uint32_t hvps_settings[3];
//	mem_read(NVM_HVPS, hvps_settings);
//
//	/* Convert these into ASCII; see memory layout diagram for details */
//	uint16_t dtp1, dtp2;
//	uint16_t dt1, dt2;
//	uint16_t v, t;
//	dtp1 = hvps_settings[0] & 0xFFFF;
//	dtp2 = (hvps_settings[0] & 0xFFFF0000) >> 16;
//	dt1 = hvps_settings[1] & 0xFFFF;
//	dt2 = (hvps_settings[1] & 0xFFFF0000) >> 16;
//	v = hvps_settings[2] & 0xFFFF;
//	t = (hvps_settings[2] & 0xFFFF0000) >> 16;
//
//	/* Compose command string, converting int16's into ASCII*/
//	sprintf(&HST[3], "%04X%04X%04X%04X%04X%04X", dtp1, dtp2, dt1, dt2, v, t);
//
//	/* Send command to HVPS if voltage check on NVM readout is successful */
//	if(voltage_check(HST) == 0)
//	{
//		prep_hvps_cmd_array(HST);
//		while(wait)
//			;
//		MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array, strlen((char *)hvps_cmd_array));
//		wait = 1;
//		cmds_sent++;
//	}
}

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


// TODO: Rename to "voltage_less_than"
// TODO: Add param for voltage value
// TODO: Remove cmd param, use hvps_cmd_array for selection...
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
	while(wait)
		;
	MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array, strlen((char *)hvps_cmd_array));
	wait = 1;
	cmds_sent++;
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
	while(wait)
		;
	MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array, strlen((char *)hvps_cmd_array));
	wait = 1;
	cmds_sent++;

	return 0;
}

void hvps_send_cmd(char *cmd)
{
	prep_hvps_cmd_array(cmd);
	while(wait)
		;
	MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd_array, strlen((char *)hvps_cmd_array));
	wait = 1;
	cmds_sent++;
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

/**
 * @brief Get the number of sent/acked/failed commands since startup
 *
 * @param c Command counter to retrieve (type `enum hvps_cmd_counter`)
 *
 * @return The number of commands of the requested type sent/acked/failed
 */
uint16_t hvps_get_cmd_counter(enum hvps_cmd_counter c)
{
	switch(c) {
		case HVPS_CMDS_SENT:
			return cmds_sent;
		case HVPS_CMDS_ACKED:
			return cmds_acked;
		case HVPS_CMDS_FAILED:
			return cmds_failed;
		default:
			return 0;
	}
}

/* UART handler for RX from HVPS */
static void UART0_RXHandler(mss_uart_instance_t * this_uart)
{
	// TODO: Make rx_buff 51-byte long, to account for max HVPS reply length.
	static uint8_t rx_buff[16]="";
	static size_t rx_size;

	rx_size += MSS_UART_get_rx(this_uart, rx_buff + rx_size, sizeof(rx_buff));
	if(rx_buff[rx_size-1] == 0x0d)
	{
		/* Clear wait flag */
		wait = 0;

		/* Increment command counters based on reply */
		if ((rx_buff[1] == hvps_cmd_array[1] + 0x20) &&
				(rx_buff[2] == hvps_cmd_array[2] + 0x20) &&
				(rx_buff[3] == hvps_cmd_array[3] + 0x20))
			cmds_acked++;
		else if(rx_buff[1] == 'h' && rx_buff[2] == 'x' && rx_buff[3] == 'x')
			cmds_failed++;

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
 * @brief Timer interrupt for sending "get" commands to the HVPS
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
