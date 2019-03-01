/*
 * main.c
 *
 *  Created on: 25 okt. 2018
 *  Author: Marcus Persson
 *  V1.0
 *  FPGA firmware code
 */

#include "msp/msp_i2c.h"
#include <stdint.h>

#include "firmware/drivers/cubes_timekeeping/cubes_timekeeping.h"
#include "firmware/drivers/citiroc/citiroc.h"

#include "hvps/hvps_c11204-02.h"
#include "msp/msp_exp.h"
#include "mem_mgmt/mem_mgmt.h"

#define SLAVE_ADDR 0x35
#define nvm_mem_addr 0x60000000

/*__attribute__((__interrupt__)) void HardFault_Handler(void){
	  __asm volatile (
	    " movs r0,#4       \n"
	    " movs r1, lr      \n"
	    " tst r0, r1       \n"
	    " beq _MSP         \n"
	    " mrs r0, psp      \n"
	    " b _HALT          \n"
	  "_MSP:               \n"
	    " mrs r0, msp      \n"
	  "_HALT:              \n"
	    " ldr r1,[r0,#20]  \n"
	    " bkpt #0          \n"
	  );
}*/

/*void delay(int ms)
{
	int d = ms * SystemCoreClock / 128;

	while (d-- > 0)
		;
}*/

int main(void){
	/*uint32_t current_time;
	uint32_t rocsr;
	uint32_t daqrdy;
	uint32_t hcr[32];
	uint32_t i;
	uint8_t data[HISTO_LEN]; */

	/*
	 * Test code for histo_ram and histo_ram_ctrl
	 *
	 * 1. Old checks, to make sure that the CSR interface still works...
	 */
/*	rocsr = citiroc_get_rocsr();
	citiroc_daq_set_dur(151);               // <--- BREAK here
	citiroc_daq_start();
	rocsr = citiroc_get_rocsr();
	citiroc_daq_stop();                     // <--- BREAK here
	daqrdy = citiroc_daq_is_rdy();

	for (i = 0; i < 32; i++) {
		hcr[i] = citiroc_get_hcr(i);
	}*/

	/* 2. Setting ROCSR.NEWCFG to '1' initiates the histo_ram memory loading
	 * process.
	 *
	 * NB: Without setting it to '1', the histo_ram will contain
	 * "garbage" data.
	 */
	CITIROC->ROCSR |= (1 << NEWSC); 		// <--- BREAK here, check CHxHCR

	 /* 3. NEWCFG should then reset on read, so check that it is '0'; */
	citiroc_get_rocsr(); 					// <--- BREAK here

	 /*
	  * 4. Wait for ROCSR.DAQRDY to be set ('1') before attempting a read.
	  * Use a 100-ms or longer delay in case it stalls...
	  * */
	while (!citiroc_daq_is_rdy())
		;


	/* Test code to read from Histogram location */
	/* First argument is memory adress, the second is a pointer to where the data is to be transferred.
	 * Length of array is defined in mem_mgmt.h
	 * */


	 /* Skip the rest for now */
	/*while (1)						// <--- BREAK here, check "daqrdy" and "hcr[x]"
		;

	mem_ram_write(RAM_HVPS, "0000000000000000746900C8");*/ /* Writing standard HVPS value to ram for testing */
	//msp_read_seqflags();
	init_i2c(SLAVE_ADDR);
	//hvps_init(nvm_mem_addr);
	while(1){
		if(has_send != 0){
			switch(has_send){
				case MSP_OP_REQ_PAYLOAD:
					break;
				case MSP_OP_REQ_HK:
					break;
				case MSP_OP_REQ_PUS:
					break;
			}
			has_send=0;
		}
		else if(has_recv != 0){
			switch(has_recv){
				case MSP_OP_SEND_TIME:
					break;
				case MSP_OP_SEND_PUS:
					break;
				case CUBES_OP_HVPS_CONF:
					hvps_set_voltage(recv_data);
					break;
				case CUBES_OP_CITI_CONF:
					break;
			}
			has_recv=0;
		}
		else if(has_syscommand != 0){
			switch(has_syscommand){
			case MSP_OP_ACTIVE:
				//hvps_turn_on();
				break;
			case MSP_OP_SLEEP:
				hvps_turn_off();
				break;
			case MSP_OP_POWER_OFF:
				hvps_turn_off();
				msp_save_seqflags();
				break;
			}
			has_syscommand=0;
		}
	}
}


