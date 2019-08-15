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
#include <stdio.h>
#include <stdlib.h>

#include "firmware/drivers/cubes_timekeeping/cubes_timekeeping.h"
#include "firmware/drivers/citiroc/citiroc.h"
#include "firmware/drivers/mss_timer/mss_timer.h"

#include "hvps/hvps_c11204-02.h"
#include "msp/msp_exp.h"
#include "mem_mgmt/mem_mgmt.h"

#define SLAVE_ADDR 0x35


int main(void)
{
	uint8_t daq_dur;

	//msp_read_seqflags();

	/* Startup delay */
	for (int i = 0; i < 1000; i++)
		;
	nvm_reset_counter_increment();
	msp_init_i2c(SLAVE_ADDR);
	hvps_init();
	//hvps_turn_off();
	while(1){
		if(has_send != 0){
			switch(has_send){
				case MSP_OP_REQ_PAYLOAD: /* Payload is transferred in msp_exp_handler.c, use this to clear memory or set citiroc bit to start new DAQ */
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
					cubes_set_time((uint32_t) strtoul((char*)msp_get_recv(), NULL, 10));
					break;
				case MSP_OP_SEND_PUS:
					break;
				case MSP_OP_SEND_CUBES_HVPS_CONF:
				{
					uint8_t turn_on = (uint8_t)msp_get_recv()[0] & 0x01;
					uint8_t hvps_resetval = (uint8_t)msp_get_recv()[0] & 0x02;

					if (turn_on && !hvps_is_on())
						hvps_send_cmd("HON");
					else if (!turn_on && hvps_is_on())
						hvps_send_cmd("HOF");
					if(hvps_resetval && hvps_is_on()){
						hvps_send_cmd("HRE");
						break;
					}

					hvps_set_temp_corr_factor(&msp_get_recv()[1]);
					hvps_send_cmd("HCM1");
					break;
				}
				case MSP_OP_SEND_CUBES_HVPS_TMP_VOLT:
				{
					uint8_t turn_on = (uint8_t)msp_get_recv()[0] & 0x01;
					uint8_t hvps_resetval = (uint8_t)msp_get_recv()[0] & 0x02;
					uint16_t volt = (uint16_t)((msp_get_recv()[1] << 8) |
											   (msp_get_recv()[2]));

					if (turn_on && !hvps_is_on())
						hvps_send_cmd("HON");
					else if (!turn_on && hvps_is_on())
						hvps_send_cmd("HOF");
					if(hvps_resetval && hvps_is_on() && turn_on)
						hvps_send_cmd("HRE");

					hvps_set_temporary_voltage(volt);

					break;
				}
				case MSP_OP_SEND_CUBES_CITI_CONF:
					mem_ram_write(RAM_CITI_CONF, msp_get_recv());
					citiroc_send_slow_control();
					break;
				case MSP_OP_SEND_CUBES_PROB_CONF:
					mem_ram_write(RAM_CITI_PROBE, msp_get_recv());
					citiroc_send_probes();
					break;
				case MSP_OP_SEND_READ_REG_DEBUG:
				{
					uint8_t rrd = msp_get_recv()[0];
					citiroc_rrd(rrd & 0x01, (rrd & 0x3e)>>1);
					break;
				}
				case MSP_OP_SEND_CUBES_DAQ_DUR:
					daq_dur = msp_get_recv()[0];
					citiroc_daq_set_dur(daq_dur);
					break;
				case MSP_OP_SEND_CUBES_GATEWARE_CONF:
				{
					uint8_t *resetvalue;
					resetvalue = msp_get_recv();
					if (resetvalue[0] & 0b00000001)
						nvm_reset_counter_reset();
					if (resetvalue[0] & 0b00000010)
						citiroc_hcr_reset();
					if (resetvalue[0] & 0b00000100)
						citiroc_histo_reset();
					if (resetvalue[0] & 0b00001000)
						citiroc_psc_reset();
					if (resetvalue[0] & 0b00010000)
						citiroc_sr_reset();
					if (resetvalue[0] & 0b00100000)
						citiroc_pa_reset();
					if (resetvalue[0] & 0b01000000)
						citiroc_trigs_reset();
					if (resetvalue[0] & 0b10000000)
						citiroc_read_reg_reset();
					break;
				}
			}
			has_recv=0;
		}
		else if(has_syscommand != 0){
			switch(has_syscommand){
			case MSP_OP_ACTIVE:
				hvps_send_cmd("HON");
				break;
			case MSP_OP_SLEEP:
				hvps_send_cmd("HOF");
				citiroc_daq_stop();
				break;
			case MSP_OP_POWER_OFF:
				hvps_send_cmd("HOF");
				citiroc_daq_stop();
				msp_save_seqflags();
				break;
			case MSP_OP_CUBES_DAQ_START:
				citiroc_hcr_reset();
				citiroc_histo_reset();
				citiroc_daq_set_hvps_temp(hvps_get_latest_temp());
				citiroc_daq_set_hvps_volt(hvps_get_latest_volt());
				citiroc_daq_set_hvps_curr(hvps_get_latest_curr());
				MSS_TIM2_load_immediate(((daq_dur-1)*100000000)&0xFFFFFFFF);
				MSS_TIM2_start();
				citiroc_daq_start();
				break;
			case MSP_OP_CUBES_DAQ_STOP:
				citiroc_daq_stop();
				break;
			}
			has_syscommand=0;
		}
	}
}

void Timer2_IRQHandler(void)
{
	citiroc_daq_set_hvps_temp(hvps_get_latest_temp());
	citiroc_daq_set_hvps_volt(hvps_get_latest_volt());
	citiroc_daq_set_hvps_curr(hvps_get_latest_curr());
	MSS_TIM2_clear_irq();
}
