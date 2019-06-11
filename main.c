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


int main(void)
{
	uint8_t daq_dur;

	/*mem_ram_write(RAM_HVPS, "0000000000000000746900C8");*/ /* Writing standard HVPS value to ram for testing */
	//msp_read_seqflags();
	init_i2c(SLAVE_ADDR);
	hvps_init(nvm_mem_addr);
	hvps_turn_off();
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
				case CUBES_OP_SEND_HVPS_CONF:
					hvps_set_voltage(msp_get_recv());
					break;
				case CUBES_OP_SEND_CITI_CONF:
					mem_ram_write(RAM_CITI_CONF, msp_get_recv());
					citiroc_send_slow_control();
					break;
				case CUBES_OP_SEND_PROB_CONF:
					mem_ram_write(RAM_CITI_PROBE, msp_get_recv());
					citiroc_send_probes();
					break;
				case CUBES_OP_SEND_DAQ_DUR_AND_START:
					daq_dur = msp_get_recv()[0];
					citiroc_daq_set_dur(daq_dur);
					// citiroc_daq_start();
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


