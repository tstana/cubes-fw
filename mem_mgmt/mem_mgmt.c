/**
 * @file mem_mgmt.c
 *
 *  Created on: 15 jan. 2019
 *  Copyright © 2020 Theodor Stana and Marcus Persson
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
#include <string.h>
#include <stdlib.h>

#include "mem_mgmt.h"

#include "../msp/msp_exp_state.h"


void mem_ram_write(uint32_t modul, uint8_t *data){
	uint32_t length=0;
	uint32_t *addr = (uint32_t *) 0x00000000;

	switch(modul){
	case RAM_CITI_CONF:
		length=CITIROC_LEN;
		addr=(uint32_t *)(CFG_RAM + CITIROC_OFS);
		break;
	case RAM_CITI_PROBE:
		length = PROBE_LEN;
		addr=(uint32_t *)(CFG_RAM + PROBE_OFS);
		break;
	default:
		addr = (uint32_t *) modul;
		length=8;
		//return;
	}
	for(int i=0; i < length; i += 4) {
		*addr = (data[i+3] << 24) |
				(data[i+2] << 16) |
				(data[i+1] <<  8) |
				(data[i]);
		addr += 1;
	}
}

int mem_nvm_write(uint32_t modul, uint8_t *data){
	uint32_t length=0;
	uint32_t *addr = (uint32_t *)0x00000000;
	switch(modul){
	case NVM_CITIROC:
		length=CITIROC_LEN;
		addr=(uint32_t *)(NVM_ADDR+CITIROC_OFFSET+(data[length-1]*length));
		break;
	case NVM_CITIROC_CONF_NUM:
		length=CITIROC_NUM_LEN;
		addr=(uint32_t *)(NVM_ADDR+CITIROC_CONF_NUM_OFFSET);
		break;
	case NVM_SEQFLAG:
		length=SEQFLAG_LEN;
		addr=(uint32_t *)(NVM_ADDR+SEQFLAG_OFFSET);
		break;
	case NVM_RESET:
		length = 4;
		addr=(uint32_t *)(NVM_ADDR+0xFE00); /* Free offset address*/
		break;
	default:
		return -1;
	}
	nvm_status_t status = NVM_unlock((uint32_t)addr, length);
			if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status)){
				status=NVM_write((uint32_t)addr, data, length, NVM_DO_NOT_LOCK_PAGE);
				if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status))
					return 0;
				else
					return -1;
			}
			else
				return -1;
}

uint32_t mem_read(uint32_t modul, uint32_t *data)
{
	uint32_t length=0;
	uint32_t *addr;

	/* Set length and address pointer to correct module through switch */
	switch(modul){
		case NVM_CITIROC:
			length = CITIROC_LEN;
			addr=(uint32_t *)(NVM_ADDR+CITIROC_OFFSET);
			break;
		case NVM_CITIROC_CONF_NUM:
            length=CITIROC_NUM_LEN;
            addr=(uint32_t *)(NVM_ADDR+CITIROC_CONF_NUM_OFFSET);
            break;
		case NVM_RESET:
			length = 4;
			addr= (uint32_t *)(NVM_ADDR+0xFE00); /* Free offset address */
			break;
		case NVM_SEQFLAG:
			length = SEQFLAG_LEN;
			addr=(uint32_t *)(NVM_ADDR+SEQFLAG_OFFSET);
			break;
		default:
			return -1; /* If the statement isn't found, return without reading */
	}

	/*
	 * Inputs to length so far were in number of bytes; turn into number of
	 * 32-bit addresses, then copy the actual data.
	 */
	length /= 4;

	int i;
	for (i = 0; i < length; ++i)
	{
		data[i] = addr[i];
	}

	/* and return the number of bytes copied */
	return length*4;
}

void nvm_reset_counter_increment(void)
{
	uint32_t counter = 0;
	mem_read(NVM_RESET, &counter);
	counter++;
	mem_nvm_write(NVM_RESET, (uint8_t *)&counter);
}

uint32_t nvm_reset_counter_read(void)
{
	uint32_t counter = 0;
	mem_read(NVM_RESET, &counter);
	return counter;
}

void nvm_reset_counter_reset(void){
	uint8_t resetvalue[4] = {0, 0, 0, 0};
	mem_nvm_write(NVM_RESET, resetvalue);
}

nvm_status_t nvm_save_msp_seqflags(void)
{
	msp_seqflags_t seqflags = msp_exp_state_get_seqflags();
	nvm_status_t status;

	status = NVM_unlock(NVM_SEQFLAG_ADDR, sizeof(msp_seqflags_t));
	if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status)){
		status = NVM_write(NVM_SEQFLAG_ADDR,
		                   (uint8_t*)&seqflags,
		                   sizeof(msp_seqflags_t),
		                   NVM_LOCK_PAGE);
	}
	return status;
}

void nvm_restore_msp_seqflags(void)
{
	msp_seqflags_t seqflags;
	mem_read(NVM_SEQFLAG, (uint32_t *)&seqflags);
	msp_exp_state_initialize(seqflags);
}

