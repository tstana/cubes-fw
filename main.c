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

/* Op-codes from ISR callbacks */
static unsigned int has_send;
static unsigned int has_send_error = 0;
static unsigned int has_send_errorcode = 0;
static unsigned int has_recv = 0;
static unsigned int has_recv_error = 0;
static unsigned int has_recv_errorcode = 0;
static unsigned int has_syscommand = 0;

/*
 * Define the MSP send data buffer. The max number of bytes that can be sent by
 * CUBES corresponds to the histogram size in gateware.
 */
#define HK_LEN    (46)
#define ID_LEN    (25)

// TODO: Add comment
static uint16_t get_num_bins(uint8_t bin_cfg);
// TODO: Add comment
static inline void prep_payload_data();

static uint8_t *send_data;
static unsigned char send_data_payload[MEM_HISTO_LEN_GW]="";
static unsigned char send_data_hk[HK_LEN] = "";
static unsigned char cubes_id[ID_LEN];

/* Receive data, Citiroc configuration is the largest */
#define RECV_MAXLEN    (MEM_CITIROC_CONF_LEN)
static unsigned char recv_data[RECV_MAXLEN];


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
			uint32_t itsy_ram;
			uint32_t u32val = 0;
			uint16_t u16val = 0;

			switch (has_send) {

				case MSP_OP_REQ_CUBES_ID:
					sprintf((char*)cubes_id, "%s %s",__DATE__, __TIME__);
					itsy_ram = citiroc_read_id();
					to_bigendian32(cubes_id+21, itsy_ram);
					send_data = cubes_id;
					break;

				case MSP_OP_REQ_HK:
					/* Reset counter and hit counter register readouts */
					u32val = cubes_get_time();
					to_bigendian32(send_data_hk, u32val);

					u32val = mem_reset_counter_read();
					to_bigendian32(send_data_hk+4, u32val);

					u32val = citiroc_hcr_get(0);
					to_bigendian32(send_data_hk+8, u32val);
					u32val = citiroc_hcr_get(16);
					to_bigendian32(send_data_hk+12, u32val);
					u32val = citiroc_hcr_get(31);
					to_bigendian32(send_data_hk+16, u32val);
					u32val = citiroc_hcr_get(32);  // OR32
					to_bigendian32(send_data_hk+20, u32val);

					/* HVPS HK */
					u16val = hvps_get_voltage();
					send_data_hk[24] = (u16val >> 8) & 0xff;
					send_data_hk[25] = u16val & 0xff;

					u16val = hvps_get_current();
					send_data_hk[26] = (u16val >> 8) & 0xff;
					send_data_hk[27] = u16val & 0xff;

					u16val = hvps_get_temp();
					send_data_hk[28] = (u16val >> 8) & 0xff;
					send_data_hk[29] = u16val & 0xff;

					u16val = hvps_get_status();
					send_data_hk[30] = (u16val >> 8) & 0xff;
					send_data_hk[31] = u16val & 0xff;

					u16val = hvps_get_cmd_counter(HVPS_CMDS_SENT);
					send_data_hk[32] = (u16val >> 8)  & 0xff;
					send_data_hk[33] = u16val  & 0xff;

					u16val = hvps_get_cmd_counter(HVPS_CMDS_ACKED);
					send_data_hk[34] = (u16val >> 8)  & 0xff;
					send_data_hk[35] = u16val  & 0xff;

					u16val = hvps_get_cmd_counter(HVPS_CMDS_FAILED);
					send_data_hk[36] = (u16val >> 8)  & 0xff;
					send_data_hk[37] = u16val  & 0xff;

					u16val = hvps_get_last_cmd_err();
					send_data_hk[38] = (u16val >> 8) & 0xff;
					send_data_hk[39] = u16val & 0xff;

					/* On-board ADC HK */
					u16val = hk_adc_calc_avg_voltage();
					send_data_hk[40] = (u16val >> 8) & 0xff;
					send_data_hk[41] = u16val & 0xff;

					u16val = hk_adc_calc_avg_current();
					send_data_hk[42] = (u16val >> 8) & 0xff;
					send_data_hk[43] = u16val & 0xff;

					u16val = hk_adc_calc_avg_citi_temp();
					send_data_hk[44] = (u16val >> 8) & 0xff;
					send_data_hk[45] = u16val & 0xff;

					/* Finally, prep the data to be sent */
					send_data = send_data_hk;
					break;

				case MSP_OP_REQ_PAYLOAD:
					prep_payload_data();
					send_data = send_data_payload;
					break;
			}

			has_send = 0;

		} else if (has_recv != 0) {

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

		} else if (has_syscommand != 0) {
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
					 * Prep. the timer used to store HK to DAQ file one second
					 * before end of DAQ. The 64-bit timer_load_value is split
					 * into two 32-bit numbers because TIM64 needs its
					 * parameters that way.
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
 *==============================================================================
 * MSP Callbacks
 *==============================================================================
 */

/*
 * ---------------------------
 * Histogram re-binning tables
 * ---------------------------
 */
static uint16_t *table;

/* define logscale tables */
static uint16_t table1[1025] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
	33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
	49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
	81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
	97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
	113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
	129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,
	145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
	161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
	177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,
	193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,
	209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,
	225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,
	241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,256,
	257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,
	273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,
	289,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,
	305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,
	321,322,323,324,325,326,327,328,329,330,331,332,333,334,335,336,
	337,338,339,340,341,342,343,344,345,346,347,348,349,350,351,352,
	353,354,355,356,357,358,359,360,361,362,363,364,365,366,367,368,
	369,370,371,372,373,374,375,376,377,378,379,380,381,382,383,384,
	385,386,387,388,389,390,391,392,393,394,395,396,397,398,399,400,
	401,402,403,404,405,406,407,408,409,410,411,412,413,414,415,416,
	417,418,419,420,421,422,423,424,425,426,427,428,429,430,431,432,
	433,434,435,436,437,438,439,440,441,442,443,444,445,446,447,448,
	449,450,451,452,453,454,455,456,457,458,459,460,461,462,463,464,
	465,466,467,468,469,470,471,472,473,474,475,476,477,478,479,480,
	481,482,483,484,485,486,487,488,489,490,491,492,493,494,495,496,
	497,498,499,500,501,502,503,504,505,506,507,508,509,510,511,512,
	514,516,518,520,522,524,526,528,530,532,534,536,538,540,542,544,
	546,548,550,552,554,556,558,560,562,564,566,568,570,572,574,576,
	578,580,582,584,586,588,590,592,594,596,598,600,602,604,606,608,
	610,612,614,616,618,620,622,624,626,628,630,632,634,636,638,640,
	642,644,646,648,650,652,654,656,658,660,662,664,666,668,670,672,
	674,676,678,680,682,684,686,688,690,692,694,696,698,700,702,704,
	706,708,710,712,714,716,718,720,722,724,726,728,730,732,734,736,
	738,740,742,744,746,748,750,752,754,756,758,760,762,764,766,768,
	770,772,774,776,778,780,782,784,786,788,790,792,794,796,798,800,
	802,804,806,808,810,812,814,816,818,820,822,824,826,828,830,832,
	834,836,838,840,842,844,846,848,850,852,854,856,858,860,862,864,
	866,868,870,872,874,876,878,880,882,884,886,888,890,892,894,896,
	898,900,902,904,906,908,910,912,914,916,918,920,922,924,926,928,
	930,932,934,936,938,940,942,944,946,948,950,952,954,956,958,960,
	962,964,966,968,970,972,974,976,978,980,982,984,986,988,990,992,
	994,996,998,1000,1002,1004,1006,1008,1010,1012,1014,1016,1018,1020,
	1022,1024,1028,1032,1036,1040,1044,1048,1052,1056,1060,1064,1068,1072,
	1076,1080,1084,1088,1092,1096,1100,1104,1108,1112,1116,1120,1124,1128,
	1132,1136,1140,1144,1148,1152,1156,1160,1164,1168,1172,1176,1180,1184,
	1188,1192,1196,1200,1204,1208,1212,1216,1220,1224,1228,1232,1236,1240,
	1244,1248,1252,1256,1260,1264,1268,1272,1276,1280,1284,1288,1292,1296,
	1300,1304,1308,1312,1316,1320,1324,1328,1332,1336,1340,1344,1348,1352,
	1356,1360,1364,1368,1372,1376,1380,1384,1388,1392,1396,1400,1404,1408,
	1412,1416,1420,1424,1428,1432,1436,1440,1444,1448,1452,1456,1460,1464,
	1468,1472,1476,1480,1484,1488,1492,1496,1500,1504,1508,1512,1516,1520,
	1524,1528,1532,1536,1540,1544,1548,1552,1556,1560,1564,1568,1572,1576,
	1580,1584,1588,1592,1596,1600,1604,1608,1612,1616,1620,1624,1628,1632,
	1636,1640,1644,1648,1652,1656,1660,1664,1668,1672,1676,1680,1684,1688,
	1692,1696,1700,1704,1708,1712,1716,1720,1724,1728,1732,1736,1740,1744,
	1748,1752,1756,1760,1764,1768,1772,1776,1780,1784,1788,1792,1796,1800,
	1804,1808,1812,1816,1820,1824,1828,1832,1836,1840,1844,1848,1852,1856,
	1860,1864,1868,1872,1876,1880,1884,1888,1892,1896,1900,1904,1908,1912,
	1916,1920,1924,1928,1932,1936,1940,1944,1948,1952,1956,1960,1964,1968,
	1972,1976,1980,1984,1988,1992,1996,2000,2004,2008,2012,2016,2020,2024,
	2028,2032,2036,2040,2044,2048
};

static uint16_t table2[129] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
	34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,
	68,72,76,80,84,88,92,96,100,104,108,112,116,120,
	124,128,136,144,152,160,168,176,184,192,200,208,
	216,224,232,240,248,256,272,288,304,320,336,352,
	368,384,400,416,432,448,464,480,496,512,544,576,
	608,640,672,704,736,768,800,832,864,896,928,960,
	992,1024,1088,1152,1216,1280,1344,1408,1472,1536,
	1600,1664,1728,1792,1856,1920,1984,2048
};


/*
 * ------------------------------------
 * Prepare data for MSP_OP_PREQ_PAYLOAD
 * ------------------------------------
 */
static uint16_t get_num_bins(uint8_t bin_config)
{
	uint16_t n;

	switch (bin_config) {
	case 0:
		n = MEM_HISTO_NUM_BINS_GW;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		n = MEM_HISTO_NUM_BINS_GW >> bin_config;
		break;
	case 10:
		n = (sizeof(table1)/sizeof(table1[0])) - 1;
		break;
	case 11:
		n = (sizeof(table2)/sizeof(table2[0])) - 1;
		break;
	default:
		n = 0;
		break;
	}

	return n;
}


static inline void prep_payload_data()
{
	unsigned long i, j, k;
	uint16_t num_bins;
	uint8_t bin_size;
	uint32_t bin;

	unsigned long send_idx; // start index used for send data payload
	unsigned long data_idx; // Index used to access data form histo_data

	uint32_t *histo_data = (uint32_t *)HISTO_RAM;

	send_idx = 0;

	/* histogram header into send_data_payload */
	for (i = 0; i < MEM_HISTO_HDR_LEN/4; i++) {
		send_data_payload[send_idx + 1] = histo_data[i] & 0xFF;
		send_data_payload[send_idx + 0] = histo_data[i]>>8 & 0xFF;
		send_data_payload[send_idx + 3] = histo_data[i]>>16 & 0xFF;
		send_data_payload[send_idx + 2] = histo_data[i]>>24 & 0xFF;
		send_idx += 4;
	}

	for (i = 0; i < 6; i++) {
		// data index for next histogram; /2 and /4  because histo_data is
		// uint32_t, histogram bins are 2 bytes wide and HDR_LEN is in bytes
		data_idx = i*MEM_HISTO_NUM_BINS_GW/2 + MEM_HISTO_HDR_LEN/4;

		num_bins = get_num_bins(bin_cfg[i]);

		if (bin_cfg[i] == 0) {
			/* No re-binning */
			for (j=0; j < MEM_HISTO_NUM_BINS_GW/2; j++) {
				send_data_payload[send_idx + 1] = histo_data[data_idx] & 0xFF;
				send_data_payload[send_idx] = histo_data[data_idx]>>8 & 0xFF;
				send_data_payload[send_idx + 3] = histo_data[data_idx]>>16 & 0xFF;
				send_data_payload[send_idx + 2] = histo_data[data_idx]>>24 & 0xFF;
				send_idx += 4;
				data_idx++;
			}
		} else if (bin_cfg[i] < 7) {
			/* Re-binning with equal interval bins */
			bin_size = 1 << bin_cfg[i];
			for (j=0; j<num_bins; j++) {
				bin = 0;
				for (k=j*bin_size/2; k<(j+1)*bin_size/2; k++) {
					bin += (histo_data[data_idx]>>16 & 0xFFFF) +
					       (histo_data[data_idx] & 0xFFFF);
					data_idx++;
				}
				bin >>= bin_cfg[i];
				send_data_payload[send_idx + 1] = bin & 0xFF;
				send_data_payload[send_idx] = (bin>>8) & 0xFF;
				send_idx += 2;
			}
		} else if ((11 <= bin_cfg[i]) && (bin_cfg[i] <= 12)) {
			/* Log-scale binning */

			/*
			 * Carry-over basically signals the previous bin ended at a
			 * half-word (bit 16) within a 32-bit word.
			 */
			uint8_t carry_over = 0;

			/* Start by selecting the appropriate table array */
			if (bin_cfg[i] == 11) {
				table = table1;
			} else if (bin_cfg[i] == 12) {
				table = table2;
			}

			/* Prepare new bin */
			bin = 0;

			for (j = 0; j < num_bins; j++) {
				/* Get next bin size by subtracting subsequent val's in table */
			    bin_size = *(table+j+1) - *(table+j);

				if ((carry_over == 0) && (bin_size%2 == 0)) {
					/*
					 * Re-binned data ends on 32-bit boundary and even number of
					 * sub-bins: new bin is simple average.
					 */
					carry_over = 0;
					for (k = *(table+j)/2; k < *(table+j+1)/2; k++) {
						bin += (histo_data[data_idx]>>16 & 0xFFFF) +
						       (histo_data[data_idx] & 0xFFFF);
						data_idx++;
					}
					bin /= bin_size;
				} else if ((carry_over == 0) && (bin_size%2 == 1)) {
					/*
					 * Re-binned data starts at word boundary and odd number of
					 * sub-bins means it will end on half-word boundary: next
					 * bin will start at half-word boundary, i.e., carry over.
					 */
					carry_over = 1;

					if (bin_size == 1) {
						bin = histo_data[data_idx] & 0xFFFF;
					} else {
						for (k = *(table+j)/2; k < (*(table+j+1)-1)/2; k++) {
							bin += (histo_data[data_idx]>>16 & 0xFFFF) +
							       (histo_data[data_idx] & 0xFFFF);
							data_idx++;
						}
						bin += (histo_data[data_idx] & 0xFFFF);
						bin /= bin_size;
					}
				} else if ((carry_over == 1) && (bin_size%2 == 0)) {
					/*
					 * Re-binned data starts at half-word boundary and will end
					 * at half-word boundary (even number of sub-bins): next bin
					 * will start at half-word and carry over.
					 */
					carry_over = 1;
					bin = histo_data[data_idx]>>16 & 0xFFFF;
					data_idx++;
					for (k = (*(table+j)+1)/2; k < (*(table+j+1)-1)/2; k++) {
						bin += (histo_data[data_idx]>>16 & 0xFFFF) +
						       (histo_data[data_idx] & 0xFFFF);
						data_idx++;
					}
					bin += (histo_data[data_idx] & 0xFFFF);
					bin /= bin_size;
				} else if ((carry_over == 1) && (bin_size%2 == 1)) {
					/*
					 * Re-binned data starts at half-word bounadry and will end
					 * at word boundary (odd number of sub-bins): new bin starts
					 * at half-word and will not carry over.
					 */
					carry_over = 0;

					bin = histo_data[data_idx]>>16 & 0xFFFF;
					data_idx++;
					if (bin_size != 1) {
						for (k = (*(table+j)+1)/2; k < *(table+j+1)/2; k++) {
							bin += (histo_data[data_idx]>>16 & 0xFFFF) +
							       (histo_data[data_idx] & 0xFFFF);
							data_idx++;
						}
						bin /= bin_size;
					}
				}

				/* Copy re-binned data in big-endian format to payload data */
				send_data_payload[send_idx + 1] = bin & 0xFF;
				send_data_payload[send_idx] = bin>>8 & 0xFF;
				send_idx += 2;
			}
		}
	}

	/* Set the `bin_cfg` fields in the Histo-RAM header */
	for (i = 0; i < 6; i++) {
		send_data_payload[MEM_HISTO_HDR_LEN - 6 + i] = bin_cfg[i];
	}

	/* Add configuration ID to Histo-RAM header */
	send_data_payload[249] = citiroc_conf_id;
}


/*
 * -------------------------
 * Experiment Send Callbacks
 * -------------------------
 */
void msp_expsend_start(unsigned char opcode, unsigned long *len)
{
	unsigned long l, i;
	// TODO: Really need to check for citiroc_daq_is_rdy()?
	if (opcode == MSP_OP_REQ_PAYLOAD && citiroc_daq_is_rdy()) {
		l = MEM_HISTO_HDR_LEN;
		for (i = 0; i < 6; ++i)
			l += 2 * get_num_bins(bin_cfg[i]);
	} else if(opcode == MSP_OP_REQ_HK) {
		l = HK_LEN;
	} else if(opcode == MSP_OP_REQ_CUBES_ID) {
		l = ID_LEN;
	} else {
		l = 0;
	}
	*len = l;
	has_send = opcode;
}


void msp_expsend_data(unsigned char opcode,
                      unsigned char *buf,
                      unsigned long len,
                      unsigned long offset)
{
	for(unsigned long i = 0; i<len; i++) {
		buf[i] = send_data[offset+i];
	}
}


void msp_expsend_complete(unsigned char opcode)
{
	if(opcode == MSP_OP_REQ_PAYLOAD)
		memset(send_data_payload, '\0', sizeof(send_data_payload));
}


void msp_expsend_error(unsigned char opcode, int error)
{
	has_send_error = opcode;
	has_send_errorcode = error;
}


/*
 * ----------------------------
 * Experiment Receive Callbacks
 * ----------------------------
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

