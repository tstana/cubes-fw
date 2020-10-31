/*
 * main.c
 *
 *  Created on: 25 okt. 2018
 *  Author: Marcus Persson
 *  V1.0
 *  FPGA firmware code
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "firmware/drivers/mss_timer/mss_timer.h"
#include "msp/msp_i2c.h"
#include "hvps/hvps_c11204-02.h"
#include "mem_mgmt/mem_mgmt.h"



int main(void)
{
	//msp_read_seqflags();

	/* Startup delay */
	for (int i = 0; i < 1000; i++)
		;

	/*
	 * Initialize peripherals for use by MSP and associated commands:
	 * 	    - I2C, for the communication link
	 * 	    - Timer2, the timer to be set to "DAQ Duration - 1 s", used to
	 * 	      write new HVPS readouts to the Citiroc HVPSR and TEMPR before
	 * 	      DAQ ends.
	 */
	msp_init_i2c(MSP_EXP_ADDR);

	MSS_TIM2_init(MSS_TIMER_ONE_SHOT_MODE);
	MSS_TIM2_load_immediate(0xffffffff);
	MSS_TIM2_enable_irq();

	/* Other init... */
	nvm_reset_counter_increment();
	hvps_init();

	/* Infinite loop */
	while(1){
		/*
		 * The "switch" statement below shows which REQ commands CUBES replies
		 * to over MSP.
		 *
		 * Code that handles preparing the send data can be found in the
		 * function "msp_expsend_start()", in the file "msp_exp_handler.c"
		 */
		if(has_send != 0){
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
		else if(has_recv != 0){
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
				case MSP_OP_SEND_CUBES_DAQ_DUR:
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
		else if(has_syscommand != 0){
			switch(has_syscommand){
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
}
