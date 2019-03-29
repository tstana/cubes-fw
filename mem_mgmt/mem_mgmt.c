/*
 * ram_mgmt.c
 *
 *  Created on: 15 jan. 2019
 *      Author: Marcus Persson
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../drivers/mss_nvm/mss_nvm.h"
#include "mem_mgmt.h"


void mem_ram_write(uint32_t modul, uint8_t *data){
	uint32_t length=0;
	uint32_t *addr = (uint32_t *) 0x60000000;

	switch(modul){
	case RAM_HVPS:
		length=HVPS_LEN;
		addr=(uint32_t *)(CFG_RAM);
		break;
	case RAM_CITIROC:
		length=CITIROC_LEN;
		addr=(uint32_t *)(CFG_RAM + 0x0C);	// TODO: Fix blunt offset assignment
		break;
	case RAM_CITI_PROB:
		length = PROBE_LEN;
		addr=(uint32_t *)(CFG_RAM + 0x0C);
	default:
		addr = (uint32_t *) modul;
		length=8;
		//return;
	}
	for(int i=0; i<length/4; i++){
		/*addr[i]<<24 = data[i+0] & 0xFF;
		addr[i]<<16 = data[i+1] & 0xFF;
		addr[i]<<8  = data[i+2] & 0xFF;
		addr[i]<<0  = data[i+3] & 0xFF;*/
		for (unsigned char j = 0; j < 4; j++) {
			addr[i] = addr[i] << 8;
			addr[i] += data[(i*4)+j];
		}
	}
}

int mem_nvm_write(uint32_t modul, uint8_t *data){
	uint32_t length=0;
	uint32_t *addr = (uint32_t *)0x00000000;
	switch(modul){
		case NVM_HVPS:
		length=HVPS_LEN;
		addr=(uint32_t *)(nvm_addr+hvps_offset);
		break;
	case NVM_CITIROC:
		length=CITIROC_LEN;
		addr=(uint32_t *)(nvm_addr+citiroc_offset);
		break;
	case NVM_SEQFLAG:
		length=SEQFLAG_LEN;
		addr=(uint32_t *)(nvm_addr+seqflag_offset);
		break;
	default:
		return -1;
	}
	nvm_status_t status = NVM_unlock((uint32_t)addr, length);
			if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status)){
				status=NVM_write((uint32_t)addr, data, length, NVM_LOCK_PAGE);
				if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status))
					return 0;
				else
					return -1;
			}
			else
				return -1;
}

void mem_read(uint32_t modul, uint32_t **data){
	uint32_t length=0;
	uint32_t *addr;
	switch(modul){ /* Set length and address pointer to correct module through switch */
		case NVM_HVPS:
			length=HVPS_LEN;
			addr=(uint32_t *)(nvm_addr+hvps_offset);
			break;
		case NVM_CITIROC:
			length=CITIROC_LEN;
			addr=(uint32_t *)(nvm_addr+citiroc_offset);
			break;
		// TODO: Gateware currently does not allow reading the CFG_RAM. Remove?
		case RAM_HVPS:
			length=HVPS_LEN;
			addr=(uint32_t *)(ram_addr+hvps_offset);
			break;
		case NVM_SEQFLAG:
			length=SEQFLAG_LEN;
			addr=(uint32_t *)(nvm_addr+seqflag_offset);
			break;
		case RAM_HISTO:
			length = HISTO_LEN;
			addr=(uint32_t *)(histo_addr);
			break;
		default:
			return; /* If the statement isn't found, return without reading */
	}
	*data  = addr;
}
