/*
 *==============================================================================
 * CUBES - Citiroc configuration functions
 *
 * Header file
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

#ifndef _CITIROC_H_
#define _CITIROC_H_

#include <stdint.h>
#include "../cubes_hw_platform.h"

/** Citiroc CSR struct definition */
struct citiroc_csr {
	uint32_t	ROCSR;
	uint32_t	TEMPR;
	uint32_t	CH0HCR;
	uint32_t	CH1HCR;
	uint32_t	CH2HCR;
	uint32_t	CH3HCR;
	uint32_t	CH4HCR;
	uint32_t	CH5HCR;
	uint32_t	CH6HCR;
	uint32_t	CH7HCR;
	uint32_t	CH8HCR;
	uint32_t	CH9HCR;
	uint32_t	CH10HCR;
	uint32_t	CH11HCR;
	uint32_t	CH12HCR;
	uint32_t	CH13HCR;
	uint32_t	CH14HCR;
	uint32_t	CH15HCR;
	uint32_t	CH16HCR;
	uint32_t	CH17HCR;
	uint32_t	CH18HCR;
	uint32_t	CH19HCR;
	uint32_t	CH20HCR;
	uint32_t	CH21HCR;
	uint32_t	CH22HCR;
	uint32_t	CH23HCR;
	uint32_t	CH24HCR;
	uint32_t	CH25HCR;
	uint32_t	CH26HCR;
	uint32_t	CH27HCR;
	uint32_t	CH28HCR;
	uint32_t	CH29HCR;
	uint32_t	CH30HCR;
	uint32_t	CH31HCR;
};

typedef struct citiroc_csr citiroc_csr_t;

/** Citiroc CSR bit fields definition */
#define NEWSC			( 0)
#define ASICPRBEN		( 1)
#define DAQZEROSUP		( 2)
#define DAQDUR			( 3)
#define DAQSTART		(11)
#define DAQSTOP			(12)
#define FORCETRIG		(13)
#define CHXHCR_RST		(14)
#define SCBUSY			(16)
#define DAQRDY			(17)
#define HISTOFULL		(18)

/** Base address and register definitions*/
#define CITIROC_CSR_BASE	CITIROC_INTF

#define CITIROC_CSR			((citiroc_csr_t *) CITIROC_CSR_BASE)	// formal
#define CITIROC				CITIROC_CSR								// convenient

/** Function definitions */
/* TODO: return something?
 * TODO: citiroc_init params (cfg_ram_base, histo_ram_base, etc.)
 * TODO: implem. readout and send config functions here?
 */
void 		citiroc_init();

uint32_t 	citiroc_get_rocsr();

void 		citiroc_daq_set_dur(uint8_t duration);
void		citiroc_daq_start();
void		citiroc_daq_stop();
uint32_t	citiroc_daq_is_rdy();

uint32_t	citiroc_hcr_get(uint32_t channel_num);
void		citiroc_hcr_reset(void);

void		citiroc_send_slow_control();
void		citiroc_send_probes();

#endif // _CITIROC_H_
