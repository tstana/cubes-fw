/*
 * main.c
 *
 *  Created on: 25 okt. 2018
 *  Author: Marcus Persson
 *  V1.0
 *  FPGA MIST I2C communication code
 */

#include <stdint.h>
#include "hvps/hvps_c11204-02.h"
#include "msp/msp_i2c_int.h"
#include "msp/msp_exp.h"
#include "ram_mgmt/ram_mgmt.h"
#define SLAVE_ADDR 0x35
#define nvm_mem_addr  NULL /* TODO: Update to memory position for NVM memory */
#define ram_mem_addr  NULL /* TODO: Update to memory position for RAM memory */

int main(void){
	init_i2c(SLAVE_ADDR);
	hvps_init(nvm_mem_addr);
	ram_init(ram_mem_addr);
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
				break;
			}
			has_syscommand=0;
		}
	}
}

/*
 * TODO: main
 * Find a way to format data to send to I2C
 *
 */
