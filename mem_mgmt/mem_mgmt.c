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
#define ram_addr 0x20000000u
#define nvm_addr 0x60000000u
#define hvps_offset 0x3000u
#define citiroc_offset 0x3008u

static unsigned int *citiroc_ram_loc =(unsigned int *) (ram_addr+citiroc_offset);
static unsigned int *hvps_ram_loc = (unsigned int *) (ram_addr+hvps_offset);

int mem_ram_citiroc_write(uint8_t *data){
	if(strlen(data)<(0x3FFFu-citiroc_offset)){ /* Control to prevent RAM size overflow */
		for(int i=0; i<128; i++){
			citiroc_ram_loc[i] = data[i];
		}
		return 0;
	}
	else
		return -1;
}
void mem_ram_citiroc_read(uint8_t *data){ /* Probably not needed, should be fabric function as defined by ICD for citiroc */
	for(int i=0; i<128; i++){
		data[i]=citiroc_ram_loc[i];
	}
}

int mem_ram_hvps_write(uint8_t* data){
	if(strlen(data)<(0x3FFFu-hvps_offset)){
		for(int i=0; i<24; i++){
			hvps_ram_loc[i] = data[i];
		}
		return 0;
	}
	else
		return -1;
}

void mem_ram_hvps_read(uint8_t* data){
	for(int i=0; i<24; i++){
		data[i] = hvps_ram_loc[i];
	}
}

void mem_nvm_read(uint32_t *addr, uint8_t *data, uint32_t length){
	for(int i=0; i<length; i++){
		data[i] = addr[i];
	}
}

int mem_nvm_hvps_write(uint8_t *data){
	uint8_t* ram_cmp = (uint8_t*) (nvm_addr+hvps_offset);
	for(int i=0; i<24; i++){
		if(data[i] != ram_cmp[i]){
			return -1;
		}
	}
	nvm_status_t status = NVM_unlock(nvm_addr, 24);
		if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status)){
			status=NVM_write(nvm_addr, data, 24, NVM_LOCK_PAGE);
			if((NVM_SUCCESS == status)||(NVM_WRITE_THRESHOLD_WARNING == status))
				return 0;
			else
				return -1;
		}
		else
			return -1;
}
