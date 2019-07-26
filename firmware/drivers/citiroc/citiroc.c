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
	/* Reset DAQDUR bits and apply new setting */
	CITIROC->ROCSR &= ~(0xFF << DAQDUR);
	CITIROC->ROCSR |= (duration << DAQDUR);
}

uint8_t citiroc_daq_get_dur()
{
	return (CITIROC->ROCSR >> DAQDUR) & 0xFF;
}

void citiroc_daq_start()
{
	/* DAQSTART bit self-clears on read. Use citiroc_daq_is_rdy() to get info DAQ status. */
	CITIROC->ROCSR |= (1 << DAQSTART);
}

void citiroc_daq_stop()
{
	/* DAQSTOP bit self-clears on read. Use citiroc_daq_is_rdy() to get info DAQ status. */
	CITIROC->ROCSR |= (1 << DAQSTOP);
}

uint32_t citiroc_daq_is_rdy()
{
	return ( (CITIROC->ROCSR & (1 << DAQRDY)) ? 1 : 0 );
}

uint32_t citiroc_get_rocsr()
{
	return CITIROC->ROCSR;
}

uint32_t citiroc_hcr_get(uint32_t channel_num)
{
	/* each channel is at "channel_num" offset from CH0HCR in memory...*/
	return *(&(CITIROC->CH0HCR) + channel_num);
}

void citiroc_hcr_reset()
{
	CITIROC->ROCSR |= (1 << CHXHCR_RST);
	// No clearing needed, bit self-resets.
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

void citiroc_histo_reset()
{
	CITIROC->ROCSR |= (1 << HISTRST);
	while (CITIROC->ROCSR & (1 << HISTRSTONGO))
		;
}
