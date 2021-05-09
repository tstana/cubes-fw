/*
 * Main file of CUBES Cortex-M3 Firmware
 *
 * Copyright © 2020 Theodor Stana and Marcus Persson
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
#include "msp/msp_i2c.h"
#include "hvps/hvps_c11204-02.h"
#include "mem_mgmt/mem_mgmt.h"
#include "hk_adc/hk_adc.h"

//for trial
#include "msp/msp_exp_handler.h"

extern unsigned int has_recv;
extern unsigned int has_send;
extern unsigned int has_syscommand;

//for trial
float result = 0;
unsigned long length= 0;



/**
 * @brief Main function, entry point of C code upon MSS reset
 * @return -1 if the infinite loop stops for some reason
 */
int main(void)
{
	nvm_reset_counter_increment();

//	nvm_restore_msp_seqflags();

	msp_i2c_init(MSP_EXP_ADDR);

	hvps_init();

	//testing
	hk_adc_return_t err = 0;
	err = hk_adc_init();
	if (err == HK_ADC_NO_ERR)
	{
	    msp_expsend_start(MSP_OP_REQ_HK, &length);
	    err = hk_adc_conv_read_volt(&result);
        err = hk_adc_conv_read_curr(&result);
        err = hk_adc_conv_read_citi_temp(&result);

        result = hk_adc_get_avg_volt();
        result = hk_adc_get_avg_curr();
        result = hk_adc_get_avg_citi_temp();

        result = hk_adc_get_avg_volt();
        result = hk_adc_get_avg_curr();
        result = hk_adc_get_avg_citi_temp();

        result = hk_adc_get_avg_volt();
        result = hk_adc_get_avg_curr();
        result = hk_adc_get_avg_citi_temp();

        if (err == HK_ADC_NO_ERR)
        {
            // Values read successfully
            err = HK_ADC_NO_ERR;
        }
	}

//	/* Init timer to write HK before DAQ end */
	MSS_TIM1_init(MSS_TIMER_ONE_SHOT_MODE);
	MSS_TIM1_load_immediate(0xffffffff);
	MSS_TIM1_enable_irq();
	NVIC_SetPriority(Timer1_IRQn, 1);

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

	return -1;
}
