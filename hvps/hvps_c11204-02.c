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


/*
 * =============================================================================
 *  Local Function Prototypes
 * =============================================================================
 */
static void UART0_RXHandler(mss_uart_instance_t* this_uart);
static int send_cmd_and_check_reply(char* cmd);
static int voltage_less_than(double v);

/*
 * =============================================================================
 *  Local Variables
 * =============================================================================
 */
static char hvps_cmd[36];
static uint16_t cmds_sent = 0;
static uint16_t cmds_acked = 0;
static uint16_t cmds_failed = 0;

static char hvps_reply[51];
static volatile uint8_t hvps_reply_ready = 0;


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
//	if(voltage_less_than(HST) == 0)
//	{
//		prep_hvps_cmd_array(HST);
//		while(wait)
//			;
//		MSS_UART_polled_tx(&g_mss_uart0, hvps_cmd, strlen((char *)hvps_cmd));
//		wait = 1;
//		cmds_sent++;
//	}

}

/**
 * @Brief Turn HV output on
 *
 * This function sends an `HON` command over UART to the C11204-02 MPPC bias
 * module to turn the HV output on.
 *
 * @return 0 if the MPPC bias module acknowledges the command (reply `hon`)
 *         1 if any other reply than `hon` was received from the HVPS
 */
int hvps_turn_on()
{
	return send_cmd_and_check_reply("HON");
}


/**
 * @Brief Turn HV output off
 *
 * This function sends an `HOF` command over UART to the C11204-02 MPPC bias
 * module to turn the HV output on.
 *
 * @return 0 if the HVPS acknowledges the command (reply `hof`)
 *         1 if any other reply than `hof` was received from the HVPS
 */
int hvps_turn_off()
{
	return send_cmd_and_check_reply("HOF");
}


/**
 * @brief Reset HVPS module
 *
 * This function sends an `HRE` command over UART to the C11204-02 MPPC bias
 * module to reset it.
 *
 * @return 0 if the HVPS acknowledges the command (reply `hre`)
 *         1 if any other reply than `hre` was received from the HVPS
 */
int hvps_reset()
{
	return send_cmd_and_check_reply("HRE");
}


int hvps_set_temp_corr_factor(struct hvps_temp_corr_factor *f)
{
	char cmd[28]="HST";

	/*
	 * Compose command string, converting int16's into ASCII; check that
	 * the applied voltage is acceptable; then compose the command string
	 * and send it over UART
	 */
	sprintf(&cmd[3], "%04X%04X%04X%04X%04X%04X", f->dtp1, f->dtp2, f->dt1,
			f-> dt2, f->vb, f->tb);
	if (!voltage_less_than(55.0))
		return -1;

	return send_cmd_and_check_reply(cmd);
}

int hvps_temp_compens_en()
{
	return send_cmd_and_check_reply("HCM1");
}


int hvps_temp_compens_dis()
{
	return send_cmd_and_check_reply("HCM0");
}


int hvps_set_temporary_voltage(uint16_t vb)
{
	/* Prep command string and parameter */
	char cmd[8] = "HBV";
	sprintf(&cmd[3], "%04X", vb);

	/* Give up early if voltage is too high */
	if(!voltage_less_than(55.0))
		return -1;

	/* Attempt to send command */
	return send_cmd_and_check_reply(cmd);
}


uint16_t hvps_get_temp(void)
{
	uint16_t temp = 1; // default should not be valid reading

	if (send_cmd_and_check_reply("HGT") == 0)
		temp = strtol((char*)hvps_reply+4, NULL, 16);

	return temp;
}


uint16_t hvps_get_volt(void)
{
	uint16_t v = 0xffff; // default should not be valid reading

	if (send_cmd_and_check_reply("HGV") == 0)
		v = strtol((char*)hvps_reply+4, NULL, 16);

	return v;
}


uint16_t hvps_get_curr(void)
{
	uint16_t c = 0xffff; // default should not be valid reading

	if (send_cmd_and_check_reply("HGC") == 0)
		c = strtol((char*)hvps_reply+4, NULL, 16);

	return c;
}


int hvps_is_on(void)
{
	uint16_t status = 0xffff; // default should not be valid reading

	if (send_cmd_and_check_reply("HGS") == 0)
		status = strtol((char*)hvps_reply+4, NULL, 16);

	return (int)(status & 0x0001);
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

/*
 * =============================================================================
 *  Local functions
 * =============================================================================
 */
static int send_cmd_and_check_reply(char *cmd)
{
	/* ------------------- Step 1: Prepare command array -------------------- */
	const uint8_t STX = 0x02;
	const uint8_t ETX = 0x03;
	const uint8_t CR = '\r';

	int cmdlen = strlen((char*)cmd);

	uint16_t chksum = 0x00;
	char chkstr[3];

	int i = 0;

	/*
	 * Start with fresh command array. Memmove is then used with offset for the
	 * address because strcat did not give the proper command format to send to
	 * the HVPS.
	 */
	memset(hvps_cmd, '\0', sizeof(hvps_cmd));

	memmove(hvps_cmd, &STX, 1);
	memmove(hvps_cmd+1, cmd, cmdlen);
	memmove(hvps_cmd+1+cmdlen, &ETX, 1);
	for(i = 0; hvps_cmd[i-1] != ETX; i++){
		chksum += hvps_cmd[i];
	}
	chksum &= 0xFF; // Mask so only lower 2 bytes get sent
	sprintf(chkstr, "%02X", chksum);
	memmove(hvps_cmd+1+cmdlen+1, chkstr, 2);
	memmove(hvps_cmd+1+cmdlen+3, &CR, 1);


	/* ----------------- Step 2: Send command over UART --------------------- */
	MSS_UART_polled_tx(&g_mss_uart0, (uint8_t *)hvps_cmd, strlen(hvps_cmd));
	cmds_sent++;
	hvps_reply_ready = 0;


	/* ---------------- Step 3: Wait for reply from HVPS -------------------- */
	while (!hvps_reply_ready)
		;


	/* ---------------- Step 4: Check for correct reply --------------------- */
	if ((hvps_reply[1] != hvps_cmd[1] + 0x20) ||
			(hvps_reply[2] != hvps_cmd[2] + 0x20) ||
			(hvps_reply[3] != hvps_cmd[3] + 0x20))
		return 1;

	return 0;
}


// TODO: Remove cmd param, use hvps_cmd for selection...
static int voltage_less_than(double v)
{
	char data[4] = "";
	double val = 0;

	/* Check for which command that came to decide on array location */
	if((hvps_cmd[1]=='H' && hvps_cmd[2]=='S' && hvps_cmd[3]=='T')) {
		for(int i=0; i<4; i++){
			data[i] = hvps_cmd[i+19];
		}
	} else if(hvps_cmd[1]=='H' && hvps_cmd[2]=='B' && hvps_cmd[3]=='V') {
		for(int i=0; i<4; i++){
			data[i] = hvps_cmd[i+3];
		}
	}
	/* Convert to long and check value for limit of 55 */
	val = strtol(data, NULL, 16);
	val = val * (1.812/pow(10, 3));
	if(val > v)
		return 0;

	return 1;
}


/**
 *  @brief UART handler for RX from HVPS
 *
 *  @param this_uart Pointer to the UART instance being handled
 */
static void UART0_RXHandler(mss_uart_instance_t* this_uart)
{
	static size_t rx_size;

	rx_size += MSS_UART_get_rx(this_uart, ((uint8_t*)hvps_reply) + rx_size,
			sizeof(hvps_reply));
	if(hvps_reply[rx_size-1] == '\r')
	{
		/* Increment command counters based on reply */
		if ((hvps_reply[1] == hvps_cmd[1] + 0x20) &&
				(hvps_reply[2] == hvps_cmd[2] + 0x20) &&
				(hvps_reply[3] == hvps_cmd[3] + 0x20)) {
			cmds_acked++;
		}
		else if(hvps_reply[1] == 'h' && hvps_reply[2] == 'x' && hvps_reply[3] == 'x')
			cmds_failed++;

		/* Inform sending command function that reply is available */
		hvps_reply_ready = 1;

		/* Prep for next round of RX... */
		rx_size = 0;
	}
}
