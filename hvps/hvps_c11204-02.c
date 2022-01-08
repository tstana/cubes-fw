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


#define HVPS_MAX_VB (55.0)

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
	MSS_UART_set_rx_handler(&g_mss_uart0, UART0_RXHandler,
			MSS_UART_FIFO_FOUR_BYTES);
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


/**
 * @brief Set temperature correction factor
 *
 * This function sends an `HST` command over UART to the C11204-02 to apply
 * new voltage, current and temperature coefficients for the temperature
 * compensation functionality of the MPPC bias module.
 *
 * Note: The settings will be applied to non-volatile memory inside the HVPS,
 *       thus the settings will be kept on power-down.
 *
 * @param f Structure containing the temperature correction factors, voltage
 *          and current
 * @return -1 if the bias voltage to be applied is greater than `HVPS_MAX_VB`
 *          0 if the HVPS acknowledges the command (reply `hst`)
 *          1 if any other reply than `hst` was received from the HVPS
 */
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
	if (!voltage_less_than(HVPS_MAX_VB))
		return -1;

	return send_cmd_and_check_reply(cmd);
}


/**-
 * @brief Enable temperature compensation
 *
 * This function sends the `HCM` command with the parameter set to `1`, to
 * enable the temperature compensation functionality in the C11204-02.
 *
 * @return 0 if the HVPS acknowledges the command (reply `hcm`)
 *         1 if any other reply than `hcm` was received from the HVPS
 */
int hvps_temp_compens_en()
{
	return send_cmd_and_check_reply("HCM1");
}


/**-
 * @brief Disable temperature compensation
 *
 * This function sends the `HCM` command with the parameter set to `0`, to
 * disable the temperature compensation functionality in the C11204-02.
 *
 * @return 0 if the HVPS acknowledges the command (reply `hcm`)
 *         1 if any other reply than `hcm` was received from the HVPS
 */
int hvps_temp_compens_dis()
{
	return send_cmd_and_check_reply("HCM0");
}


/**
 * @brief Set temporary MPPC bias voltage
 *
 * This function sends an `HBV` command to the C11204-02 to apply a temporary
 * bias voltage at the HVPS's output. Temporary means that the bias voltage is
 * _not_ written to non-volatile memory (unlike the `HST` command called by the
 * `hvps_set_temp_corr_factor()` function), thus the setting will not be kept
 * after power-down.
 *
 * @param vb Bias voltage to be applied, `uint16_t` in the format expected by
 *           the HVPS, see the C11204-02 Command Reference Manual.
 * @return -1 if the voltage is less than `HVPS_MAX_VB`
 *          0 if the HVPS acknowledges the command (reply `hbv`)
 *          1 if any other reply than `hbv` was received from the HVPS
 */
int hvps_set_temporary_voltage(uint16_t vb)
{
	/* Prep command string and parameter */
	char cmd[8] = "HBV";
	sprintf(&cmd[3], "%04X", vb);

	/* Give up early if voltage is too high */
	if(!voltage_less_than(HVPS_MAX_VB))
		return -1;

	/* Attempt to send command */
	return send_cmd_and_check_reply(cmd);
}


/**
 * @brief Get temperature readout from the C11204-02
 *
 * This function will send an `HGT` command to the HVPS to retrieve the
 * temperature readout made by the HVPS.
 *
 * @return The temperature readout from the HVPS, in the format specified by the
 *         C11204-02 Command Reference Manual.
 *
 *         If the readout was not successful (the reply was not `hgv`), the
 *         value `1` is returned. This converts to 188.18 deg. C, a temperature
 *         at which the HVPS can no longer operate - thus unreachable.
 */
uint16_t hvps_get_temp(void)
{
	uint16_t temp = 1; // default should not be valid reading

	if (send_cmd_and_check_reply("HGT") == 0)
		temp = strtol((char*)hvps_reply+4, NULL, 16);

	return temp;
}


/**
 * @brief Get voltage readout from the C11204-02
 *
 * This function will send an `HGV` command to the HVPS to retrieve the
 * temperature readout made by the HVPS.
 *
 * @return The voltage readout from the HVPS, in the format specified by the
 *         C11204-02 Command Reference Manual.
 *
 *         If the readout was not successful (the reply was not `hgv`), the
 *         value `0xffff` is returned. This converts to 118.7 V, a value that
 *         cannot be produced at the output of the HVPS, thus unreachable.
 */
uint16_t hvps_get_voltage(void)
{
	uint16_t v = 0xffff; // default should not be valid reading

	if (send_cmd_and_check_reply("HGV") == 0)
		v = strtol((char*)hvps_reply+4, NULL, 16);

	return v;
}


/**
 * @brief Get current readout from the C11204-02
 *
 * This function will send an `HGC` command to the HVPS to retrieve the
 * temperature readout made by the HVPS.
 *
 * @return The current readout from the HVPS, in the format specified by the
 *         C11204-02 Command Reference Manual.
 *
 *         If the readout was not successful (the reply was not `hgc`), the
 *         value `0xffff` is returned. This converts to 340.4 mA, a current that
 *         cannot be supplied by the HVPS, thus unreachable.
 */
uint16_t hvps_get_current(void)
{
	uint16_t c = 0xffff; // default should not be valid reading

	if (send_cmd_and_check_reply("HGC") == 0)
		c = strtol((char*)hvps_reply+4, NULL, 16);

	return c;
}


/**
 * @brief Check whether the HVPS output is on
 *
 * This function requests the status of the MPPC bias module using the `HGS`
 * command and returns the status of the first bit in the reply, which indicates
 * the output status.
 *
 * @return 1 if HVPS output is _on_
 *         0 if HVPS output is _off_
 */
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

/**
 * @brief Prepare the command string to the MPPC bias module, send the command
 *        and check for correct reply
 * @param cmd Command string to send, including parameters but excluding the
 *            start, end characters and checksum
 * @return 0 if the reply to the command _is_ correct (lower-case representation
 *           of the sent command)
 *         1 if the reply to the command _is not_ correct (`hxx` or other reply)
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
	MSS_UART_polled_tx(&g_mss_uart0, (uint8_t*)hvps_cmd, strlen(hvps_cmd));
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


/**
 * @brief Check that MPPC bias voltage is less than a certain value
 *
 * This function is used to ensure that the voltage is not higher than the
 * SPM can take, preventing its destruction.
 *
 * @param v The maximum bias voltage to check against
 * @return 1 (true) if the voltage _is_ less than the parameter `v`
 *         0 (false) if voltage _is not_ less than the parameter `v`
 */
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
	static size_t rx_size = 0;

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
