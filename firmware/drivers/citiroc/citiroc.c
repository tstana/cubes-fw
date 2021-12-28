/*
 *==============================================================================
 * CUBES - Citiroc configuration functions
 *
 * Source file
 *==============================================================================
 *
 * author: Theodor Stana (stana@kth.se)
 *
 * date of creation: 2019-02-15
 *
 * description:
 * 		<TODO: Add description here...>
 *
 *==============================================================================
 * GNU LESSER GENERAL PUBLIC LICENSE
 *==============================================================================
 * This source file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version. This source is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details. You should have
 * received a copy of the GNU Lesser General Public License along with this
 * source; if not, download it from http://www.gnu.org/licenses/lgpl-2.1.html
 *==============================================================================
 */

#include "citiroc.h"

void citiroc_init()
{
	CITIROC->ROCR = 0x0000;
}

void citiroc_daq_set_dur(uint8_t duration)
{
	/* Reset DAQDUR bits and apply new setting */
	CITIROC->ROCR &= ~(0xFF << DAQDUR);
	CITIROC->ROCR |= (duration << DAQDUR);
}

uint8_t citiroc_daq_get_dur()
{
	return (CITIROC->ROCR >> DAQDUR) & 0xFF;
}

void citiroc_daq_start()
{
	/* DAQSTART bit self-clears on read. Use citiroc_daq_is_rdy() to get info DAQ status. */
	CITIROC->ROCR |= (1 << DAQSTART);
}

void citiroc_daq_stop()
{
	/* DAQSTOP bit self-clears on read. Use citiroc_daq_is_rdy() to get info DAQ status. */
	CITIROC->ROCR |= (1 << DAQSTOP);
}

uint32_t citiroc_daq_is_rdy()
{
	return ( (CITIROC->ROSR & (1 << DAQRDY)) ? 1 : 0 );
}

void citiroc_daq_set_hvps_temp(uint16_t temp)
{
	CITIROC->TEMPR &= ~(0xffff);
	CITIROC->TEMPR |= (temp & 0xffff);
}

void citiroc_daq_set_citi_temp(uint16_t citi_temp)
{
	CITIROC->TEMPR &= ~(0xffff0000);
	CITIROC->TEMPR |= ((citi_temp << 16) & 0xffff0000);
}

void citiroc_daq_set_hvps_volt(uint16_t volt)
{
	CITIROC->HVPSR &= ~(0xffff);
	CITIROC->HVPSR |= (volt & 0xffff);
}

void citiroc_daq_set_hvps_curr(uint16_t curr)
{
	CITIROC->HVPSR &= ~((0xffff)<<16);
	CITIROC->HVPSR |= ((curr & 0xffff)<<16);
}

void citiroc_calib_set(uint32_t calibration)
{
	uint32_t temp;
	temp = calibration & CALIBPER;

	CITIROC->ASICCALR &= ~(0xffffffff);

	/*For period greater than zero, apply the register*/
	if (temp != 0)
	{
		CITIROC->ASICCALR = calibration;
	}
}

uint32_t citiroc_hcr_get(uint32_t channel_num)
{
	/* each channel is at "channel_num" offset from CH0HCR in memory...*/
	return *(&(CITIROC->CH0HCR) + channel_num);
}

void citiroc_hcr_reset()
{
	CITIROC->ROCR |= (1 << RSTALLHCR);
	// No clearing needed, bit self-resets.
}

void citiroc_send_slow_control()
{
	// TODO: Add mem_write to CFG_RAM_BASE here & add data buffer as param ???
	CITIROC->ROCR |= (1 << NEWSC);
	while (CITIROC->ROSR & (1 << SCBUSY))
		;
}

void citiroc_send_probes()
{
	// TODO: Add mem_write to CFG_RAM_BASE here & add data buffer as param ???
	CITIROC->ROCR |= (1 << ASICPRBEN);
	CITIROC->ROCR |= (1 << NEWSC);
	while (CITIROC->ROSR & (1 << SCBUSY))
		;
	CITIROC->ROCR &= ~(1 << ASICPRBEN);
}

void citiroc_rrd(uint32_t enable, uint8_t chan)
{
	/* Start by clearing the existing RRD bits */
	CITIROC->ROCR &= ~(1 << RRDEN);
	CITIROC->ROCR &= ~(0x1f << RRDCHAN);

	/* Now, apply the new setting */
	CITIROC->ROCR |= ((enable ? 1 : 0) << RRDEN);
	CITIROC->ROCR |= ((chan & 0x1f) << RRDCHAN);
	CITIROC->ROCR |= (1 << RRDNEW);
}

void citiroc_histo_reset()
{
	CITIROC->ROCR |= (1 << RSTHIST);
	while (CITIROC->ROSR & (1 << HISTRSTONGO))
		;
}

void citiroc_psc_reset()
{
	uint32_t r;

	CITIROC->ROCR |= (1 << RSTPSC);
	for (r = 0; r < 16; ++r)
		;
	CITIROC->ROCR &= ~(1 << RSTPSC);
}

void citiroc_pa_reset()
{
	uint32_t r;

	CITIROC->ROCR |= (1 << RSTPA);
	for (r = 0; r < 16; ++r)
		;
	CITIROC->ROCR &= ~(1 << RSTPA);
}

void citiroc_sr_reset()
{
	uint32_t r;

	CITIROC->ROCR |= (1 << RSTSR);
	for (r = 0; r < 16; ++r)
		;
	CITIROC->ROCR &= ~(1 << RSTSR);
}

void citiroc_trigs_reset()
{
	uint32_t r;

	CITIROC->ROCR |= (1 << RSTASICTRIGS);
	for (r = 0; r < 16; ++r)
		;
	CITIROC->ROCR &= ~(1 << RSTASICTRIGS);
}

void citiroc_read_reg_reset()
{
	uint32_t r;

	CITIROC->ROCR |= (1 << RSTREADREG);
	for (r = 0; r < 16; ++r)
		;
	CITIROC->ROCR &= ~(1 << RSTREADREG);
}

uint32_t citiroc_read_id()
{
//	uint16_t boardid;
//	uint32_t gwid;
//	boardid = *(uint32_t *)(CITIROC + 0x100);
//	gwid = *(uint32_t *)(CITIROC + 0x104);
	return 0; // (((boardid & 0xff)<<24) | (gwid & 0x00ffffff)) ;
}
