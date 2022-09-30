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
		/*
		 * The "switch" statement below shows which REQ commands CUBES replies
		 * to over MSP.
		 *
		 * Code that handles preparing the send data can be found in the
		 * function "msp_expsend_start()", in the file "msp_exp_handler.c"
		 */
		if(has_send != 0) {
			switch(has_send) {
				case MSP_OP_REQ_PAYLOAD:
					break;
				case MSP_OP_REQ_HK:
					break;
			}
			has_send=0;
		}

		/*
		 * The "switch" statement below shows which SEND commands CUBES
		 * expects over MSP.
		 *
		 * Code that handles using the receive data can be found in the function
		 * "msp_exprecv_complete()", in the file "msp_exp_handler.c"
		 */
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

		/*
		 * The "switch" statement below shows which SYSTEM commands CUBES
		 * expects over MSP.
		 *
		 * Code that handles using the receive data can be found in the function
		 * "msp_exprecv_syscommand()", in the file "msp_exp_handler.c"
		 */
		else if(has_syscommand != 0) {
			switch(has_syscommand) {
				case MSP_OP_ACTIVE:
					break;
				case MSP_OP_SLEEP:
					break;
				case MSP_OP_POWER_OFF:
					break;
				case MSP_OP_CUBES_DAQ_START:
					break;
				case MSP_OP_CUBES_DAQ_STOP:
					break;
			}
			has_syscommand = 0;
		}
	}

	// This point should not be reached
	return -1;
}
