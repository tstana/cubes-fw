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

#include "hvps/hvps_c11204-02.h"
#include "msp/msp_exp.h"
#include "mem_mgmt/mem_mgmt.h"

#define SLAVE_ADDR 0x35
#define nvm_mem_addr 0x60000000

int main(void){
	uint32_t current_time;

	/*
	 * Test code: set various values of time and check that they are set.
	 *
	 * At each breakpoint, _mentally_ count the number of seconds passed between
	 * the time when the breakpoint hits and when you click "Run" in the Debug
	 * window. When the next breakpoint hits, check that the current_time value
	 * is according to the mental count. Then repeat.
	 *
	 * The rest of the function is virtually skipped for now.
	 */
	while (1) {
		current_time = cubes_get_time();
		cubes_set_time(0);
		current_time = cubes_get_time();
		cubes_set_time(60);
		current_time = cubes_get_time();
		cubes_set_time((uint32_t)0x0fffffff);
		current_time = cubes_get_time();
	}

	mem_ram_write(RAM_HVPS, "0000000000000000746900C8"); /* Writing standard HVPS value to ram for testing */
	msp_read_seqflags();
	init_i2c(SLAVE_ADDR);
	hvps_init(nvm_mem_addr);
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
				hvps_turn_on();
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


