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
	CITIROC->ROCSR = 0x0000;
}

void citiroc_daq_set_dur(uint8_t duration)
{
	/* Clear DAQDUR bits and set to new value */
	CITIROC->ROCSR &= ~(0xFF << DAQDUR);
	CITIROC->ROCSR |= (duration << DAQDUR);
}

void citiroc_daq_start()
{
	CITIROC->ROCSR |= (1 << DAQSTART);
}

void citiroc_daq_stop()
{
	CITIROC->ROCSR |= (1 << DAQSTOP);
	/* TODO: Is this really necessary: */
	CITIROC->ROCSR &= ~(1 << DAQSTART);
}

uint32_t citiroc_daq_is_rdy()
{
	return ( (CITIROC->ROCSR & (1 << DAQRDY)) ? 1 : 0 );
}

uint32_t citiroc_get_rocsr()
{
	return CITIROC->ROCSR;
}

uint32_t citiroc_get_hcr(uint32_t channel_num)
{
	/* each channel is at "channel_num" offset from CH0HCR in memory...*/
	return *(&(CITIROC->CH0HCR) + channel_num);
}

void citiroc_send_slow_control()
{
	// TODO: Add mem_write to CFG_RAM_BASE here & add data buffer as param ???
	CITIROC->ROCSR |= (1 << NEWSC);
	while (CITIROC->ROCSR & (1 << SCBUSY))
		;
}

void citiroc_send_probes()
{
	// TODO: Add mem_write to CFG_RAM_BASE here & add data buffer as param ???
	CITIROC->ROCSR |= (1 << ASICPRBEN);
	CITIROC->ROCSR |= (1 << NEWSC);
	while (CITIROC->ROCSR & (1 << SCBUSY))
		;
	CITIROC->ROCSR &= ~(1 << ASICPRBEN);
}
