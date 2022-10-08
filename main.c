/*
 * Main file of CUBES Cortex-M3 Firmware
 *
 * Copyright © 2022 Theodor Stana and Marcus Persson
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <system_m2sxxx.h>

#include "firmware/drivers/cubes_timekeeping/cubes_timekeeping.h"
#include "firmware/drivers/mss_timer/mss_timer.h"
#include "firmware/drivers/citiroc/citiroc.h"

#include "hk_adc/hk_adc.h"

#include "hvps/hvps_c11204-02.h"

#include "mem/mem.h"

#include "msp/msp_exp.h"
#include "msp/msp_i2c.h"

#include "utils/led.h"
#include "utils/timer_delay.h"

/*
 * ---------------------
 * MSP-related variables
 * ---------------------
 */

/* Receive data, Citiroc configuration is the largest */
#define RECV_MAXLEN    (MEM_CITIROC_CONF_LEN)
static unsigned char recv_data[RECV_MAXLEN];

/* Op-codes from ISR callbacks */
extern unsigned int has_send;  // !!! TODO: This will be a static !!!
static unsigned int has_recv = 0;
static unsigned int has_recv_error = 0;
static unsigned int has_recv_errorcode = 0;
static unsigned int has_syscommand = 0;


uint8_t clean_poweroff = 0;

uint8_t citiroc_conf_id;

/* DAQ-related variables */
static uint8_t daq_dur;
uint8_t bin_cfg[6];



/**
 * @brief Main function, entry point of C code upon MSS reset
 * @return -1 if the infinite loop stops for some reason
 */
int main(void)
{
	mem_reset_counter_increment();

	/* Peripheral initializations */
	timer_delay_init();

	led_init();

	hvps_init();

	hk_adc_init();

	/* Init timer to write HK before DAQ end */
	MSS_TIM64_init(MSS_TIMER_ONE_SHOT_MODE);
	MSS_TIM64_enable_irq();
	NVIC_SetPriority(Timer1_IRQn, 1);

	/*
	 * Init. MIST Space Protocol stack; use new MSP seq. flags if previous
	 * power-off was "not so clean".
	 */
	msp_i2c_init(MSP_EXP_ADDR);

	mem_read(MEM_CLEAN_POWEROFF_ADDR, 1, &clean_poweroff);

	if (clean_poweroff) {
		mem_restore_msp_seqflags();
		clean_poweroff = 0;
		mem_write_nvm(MEM_CLEAN_POWEROFF_ADDR, 1, &clean_poweroff);
	} else
		msp_exp_state_initialize(msp_seqflags_init());

    /* Flash on-board LED to indicate init. done; leave LED on after. */
	led_blink_repeat(2, 500);
	led_turn_on();

	/*
	 * Load Citiroc configuration on startup
	 */
	mem_read(MEM_CITIROC_CONF_ID_ADDR, MEM_CITIROC_CONF_ID_LEN,
				&citiroc_conf_id);
	uint32_t *nvm_conf_addr = (uint32_t*)(MEM_CITIROC_CONF_ADDR_NVM +
			(citiroc_conf_id * MEM_CITIROC_CONF_LEN));

	/* Apply configuration if there is an existing one */
	if (citiroc_conf_id == nvm_conf_addr[MEM_CITIROC_CONF_LEN-1]) {
		// TODO: Check for return value here!
		mem_write_nvm(MEM_CITIROC_CONF_ID_ADDR, MEM_CITIROC_CONF_ID_LEN,
				&citiroc_conf_id);
		// TODO: Check for return value here!
		mem_write(MEM_CITIROC_CONF_ADDR, MEM_CITIROC_CONF_LEN,
		          (uint8_t*)nvm_conf_addr);
		citiroc_send_slow_control();
	} else {
		/* TODO: Handle what happens if no config found... */
	}

	/* Infinite loop */
	while(1) {
		if (has_send != 0) {
			switch (has_send) {
				case MSP_OP_REQ_PAYLOAD:
					break;
				case MSP_OP_REQ_HK:
					break;
			}
			has_send=0;
		}

		else if (has_recv != 0) {
			switch (has_recv) {
				case MSP_OP_SEND_TIME:
					cubes_set_time((recv_data[0] << 24) |
					               (recv_data[1] << 16) |
					               (recv_data[2] <<  8) |
					               (recv_data[3]));
					break;
				case MSP_OP_SEND_CUBES_HVPS_CONF:
				{
					uint8_t turn_on = recv_data[0] & 0x01;
					uint8_t reset = (uint8_t)recv_data[0] & 0x02;

					if (turn_on && !hvps_is_on())
						hvps_turn_on();
					else if (!turn_on && hvps_is_on())
						hvps_turn_off();

					if (reset && hvps_is_on())
						hvps_reset();

					/*
					 * Apply temperature correction factor if the command was
					 * not a "turn off" or a "reset"...
					 */
					if (turn_on && !reset) {
						struct hvps_temp_corr_factor f;

						f.dtp1 = (((uint16_t)recv_data[1]) << 8) |
						          ((uint16_t)recv_data[2]);
						f.dtp2 = (((uint16_t)recv_data[3]) << 8) |
						          ((uint16_t)recv_data[4]);
						f.dt1 = (((uint16_t)recv_data[5]) << 8) |
						         ((uint16_t)recv_data[6]);
						f.dt2 = (((uint16_t)recv_data[7]) << 8) |
						         ((uint16_t)recv_data[8]);
						f.vb = (((uint16_t)recv_data[ 9]) << 8) |
						        ((uint16_t)recv_data[10]);
						f.tb = (((uint16_t)recv_data[11]) << 8) |
						        ((uint16_t)recv_data[12]);

						hvps_set_temp_corr_factor(&f);
						hvps_temp_compens_en();
					}
					break;
				}
				case MSP_OP_SEND_CUBES_HVPS_TMP_VOLT:
				{
					uint8_t turn_on = recv_data[0] & 0x01;
					uint8_t reset = recv_data[0] & 0x02;

					if (turn_on && !hvps_is_on())
						hvps_turn_on();
					else if (!turn_on && hvps_is_on())
						hvps_turn_off();
					if(reset && hvps_is_on())
						hvps_reset();

					if (turn_on && !reset)
						hvps_set_temporary_voltage((((uint16_t)recv_data[1]) << 8) |
						                            ((uint16_t)recv_data[2]));

					break;
				}
				case MSP_OP_SEND_CUBES_CITI_CONF:
					// TODO: Check that we got all of `MEM_CITIROC_CONF_LEN`?
					// TODO: Check for return code!
					mem_write(MEM_CITIROC_CONF_ADDR, MEM_CITIROC_CONF_LEN,
					          recv_data);
					citiroc_send_slow_control();
					break;
				case MSP_OP_SEND_CUBES_PROB_CONF:
					// TODO: Check for return value here!
					mem_write(MEM_CITIROC_PROBE_ADDR, MEM_CITIROC_PROBE_LEN,
					          recv_data);
					citiroc_send_probes();
					break;
				case MSP_OP_SEND_NVM_CITI_CONF:
					// TODO: Check that we got all of `MEM_CITIROC_CONF_LEN`?
					// TODO: Check for return code!
					mem_write_nvm(MEM_CITIROC_CONF_ADDR_NVM,
					              MEM_CITIROC_CONF_LEN,
					              recv_data);
					break;
				case MSP_OP_SELECT_NVM_CITI_CONF:
				{
					/* Get CONF_ID from MSP frame */
					citiroc_conf_id = (uint8_t)recv_data[0];
					uint32_t *nvm_conf_addr =
						(uint32_t*)(MEM_CITIROC_CONF_ADDR_NVM +
						            (citiroc_conf_id * MEM_CITIROC_CONF_LEN));
					/* Apply configuration if there is an existing one */
					if (citiroc_conf_id ==
					    nvm_conf_addr[MEM_CITIROC_CONF_LEN-1]) {
						// TODO: Check for return value here!
						mem_write_nvm(MEM_CITIROC_CONF_ID_ADDR,
						              MEM_CITIROC_CONF_ID_LEN,
						              &citiroc_conf_id);
						// TODO: Check for return value here!
						mem_write(MEM_CITIROC_CONF_ADDR, MEM_CITIROC_CONF_LEN,
						          (uint8_t*)nvm_conf_addr);
						citiroc_send_slow_control();
					}
					break;
				}
				case MSP_OP_SEND_READ_REG_DEBUG:
					citiroc_rrd(recv_data[0] & 0x01, (recv_data[0] & 0x3e)>>1);
					break;
				case MSP_OP_SEND_CUBES_DAQ_CONF:
					daq_dur = recv_data[0];
					memcpy(bin_cfg, recv_data+1, 6);
					// Ensure the bin configuration is not an unsupported one
					// 		!!----FIXME----!!
		//			if (bin_cfg > 3)
		//				bin_cfg = 3;
					citiroc_daq_set_dur(daq_dur);
					break;
				case MSP_OP_SEND_CUBES_GATEWARE_CONF:
				{
					uint8_t resetvalue = recv_data[0];
					if (resetvalue & 0b00000001)
						mem_reset_counter_clear();    // TODO: Check return value?
					if (resetvalue & 0b00000010)
						citiroc_hcr_reset();
					if (resetvalue & 0b00000100)
						citiroc_histo_reset();
					if (resetvalue & 0b00001000)
						citiroc_psc_reset();
					if (resetvalue & 0b00010000)
						citiroc_sr_reset();
					if (resetvalue & 0b00100000)
						citiroc_pa_reset();
					if (resetvalue & 0b01000000)
						citiroc_trigs_reset();
					if (resetvalue & 0b10000000)
						citiroc_read_reg_reset();
					break;
				}
				case MSP_OP_SEND_CUBES_CALIB_PULSE_CONF:
					citiroc_calib_set((recv_data[0] << 24) |
					                  (recv_data[1] << 16) |
					                  (recv_data[2] << 8) |
					                  (recv_data[3]));
					break;
			}
			has_recv = 0;
		}

		else if (has_syscommand != 0) {
			uint64_t timer_load_value;
			uint32_t load_value_u, load_value_l;

			switch (has_syscommand) {
				case MSP_OP_ACTIVE:
					hvps_turn_on();
					break;
				case MSP_OP_SLEEP:
					hvps_turn_off();
					citiroc_daq_stop();
					break;
				case MSP_OP_POWER_OFF:
					hvps_turn_off();
					citiroc_daq_stop();
					if (mem_save_msp_seqflags() == NVM_SUCCESS) {
						clean_poweroff = 1;
						mem_write_nvm(MEM_CLEAN_POWEROFF_ADDR, 1,
						              &clean_poweroff);
					}
					break;
				case MSP_OP_CUBES_DAQ_START:
					/* Prep. gateware for DAQ */
					citiroc_hcr_reset();
					citiroc_histo_reset();
					citiroc_daq_set_hvps_temp(hvps_get_temp());
					citiroc_daq_set_citi_temp(hk_adc_calc_avg_citi_temp());
					citiroc_daq_set_hvps_volt(hvps_get_voltage());
					citiroc_daq_set_hvps_curr(hvps_get_current());
					/*
					 *  Prep. the timer used to store HK to DAQ file one second
					 *  before end of DAQ. The 64-bit timer_load_value is split
					 *  into two 32-bit numbers because TIM64 needs its
					 *  parameters that way.
					 */
					timer_load_value = (daq_dur-1) * SystemCoreClock;
					load_value_u = timer_load_value >> 32;
					load_value_l = (uint32_t)timer_load_value;
					MSS_TIM64_load_immediate(load_value_u, load_value_l);
					/* Start timer and DAQ */
					MSS_TIM64_start();
					citiroc_daq_start();
					break;
				case MSP_OP_CUBES_DAQ_STOP:
					citiroc_daq_set_hvps_temp(hvps_get_temp());
					citiroc_daq_set_citi_temp(hk_adc_calc_avg_citi_temp());
					citiroc_daq_set_hvps_volt(hvps_get_voltage());
					citiroc_daq_set_hvps_curr(hvps_get_current());
					citiroc_daq_stop();
					MSS_TIM64_stop();
					break;
			}
			has_syscommand = 0;
		}
	}

	// This point should not be reached
	return -1;
}


/*
 *-----------------------------
 * Experiment Receive Callbacks
 *-----------------------------
 */
void msp_exprecv_start(unsigned char opcode, unsigned long len)
{
	memset(recv_data, '\0', sizeof(recv_data));
}


void msp_exprecv_data(unsigned char opcode,
                      const unsigned char *buf,
                      unsigned long len,
                      unsigned long offset)
{
	for (unsigned long i=0; i<len; i++) {
		if((i+offset) < RECV_MAXLEN)
			recv_data[i+offset] = buf[i];
		else
			break;
	}
}


void msp_exprecv_complete(unsigned char opcode)
{
	has_recv = opcode;
}


void msp_exprecv_error(unsigned char opcode, int error)
{
	has_recv_error = opcode;
	has_recv_errorcode = error;
}


/*
 * ------------------------
 * System Command Callbacks
 * ------------------------
 */
/**
 * @brief MSP callback for when a system command has been received
 *
 * @param opcode The opcode for the received system command
 */
void msp_exprecv_syscommand(unsigned char opcode)
{
	has_syscommand = opcode;
}


/*
 *==============================================================================
 * Timer64 ISR
 *==============================================================================
 */
void Timer1_IRQHandler(void)
{
	citiroc_daq_set_hvps_temp(hvps_get_temp());
	citiroc_daq_set_citi_temp(hk_adc_calc_avg_citi_temp());
	citiroc_daq_set_hvps_volt(hvps_get_voltage());
	citiroc_daq_set_hvps_curr(hvps_get_current());
	MSS_TIM64_clear_irq();
}

