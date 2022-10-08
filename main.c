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

#include <system_m2sxxx.h>

#include "firmware/drivers/mss_timer/mss_timer.h"
#include "firmware/drivers/citiroc/citiroc.h"

#include "hk_adc/hk_adc.h"

#include "hvps/hvps_c11204-02.h"

#include "mem/mem.h"

#include "msp/msp_exp.h"
#include "msp/msp_i2c.h"

#include "utils/led.h"
#include "utils/timer_delay.h"

extern unsigned int has_recv;
extern unsigned int has_send;
extern unsigned int has_syscommand;

uint8_t clean_poweroff = 0;

uint8_t citiroc_conf_id;

// local variables used for TIM64
static uint64_t timer_load_value;
static uint32_t load_value_u, load_value_l;
uint64_t temp_value;

extern uint8_t daq_dur;



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
		if(has_send != 0) {
			switch(has_send) {
				case MSP_OP_REQ_PAYLOAD:
					break;
				case MSP_OP_REQ_HK:
					break;
			}
			has_send=0;
		}

		else if(has_recv != 0) {
			switch(has_recv) {
				case MSP_OP_SEND_TIME:
					break;
				case MSP_OP_SEND_CUBES_HVPS_CONF:
					break;
				case MSP_OP_SEND_CUBES_HVPS_TMP_VOLT:
					break;
				case MSP_OP_SEND_CUBES_CITI_CONF:
					break;
				case MSP_OP_SEND_CUBES_PROB_CONF:
					break;
				case MSP_OP_SEND_READ_REG_DEBUG:
					break;
				case MSP_OP_SEND_CUBES_DAQ_CONF:
					break;
				case MSP_OP_SEND_CUBES_GATEWARE_CONF:
					break;
				case MSP_OP_SEND_CUBES_CALIB_PULSE_CONF:
					break;
			}
			has_recv=0;
		}

		else if(has_syscommand != 0) {
			switch(has_syscommand) {
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
						mem_write_nvm(MEM_CLEAN_POWEROFF_ADDR, 1, &clean_poweroff);
					}
					break;
				case MSP_OP_CUBES_DAQ_START:
					citiroc_hcr_reset();
					citiroc_histo_reset();
					citiroc_daq_set_hvps_temp(hvps_get_temp());
					citiroc_daq_set_citi_temp(hk_adc_calc_avg_citi_temp());
					citiroc_daq_set_hvps_volt(hvps_get_voltage());
					citiroc_daq_set_hvps_curr(hvps_get_current());

					timer_load_value = (uint64_t)(daq_dur-1)*(uint64_t)SystemCoreClock;
					// split the 64-bit timer_load_value into two 32-bit numbers because
					// TIM64 needs its parameters that way
					temp_value = timer_load_value & 0xFFFFFFFF00000000ULL;
					temp_value = temp_value >> 32;
					load_value_u = temp_value;
					load_value_l = (uint32_t)(timer_load_value & 0xFFFFFFFFULL);
					MSS_TIM64_load_immediate(load_value_u, load_value_l);
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

void Timer1_IRQHandler(void)
{
	citiroc_daq_set_hvps_temp(hvps_get_temp());
	citiroc_daq_set_citi_temp(hk_adc_calc_avg_citi_temp());
	citiroc_daq_set_hvps_volt(hvps_get_voltage());
	citiroc_daq_set_hvps_curr(hvps_get_current());
	MSS_TIM64_clear_irq();
}

