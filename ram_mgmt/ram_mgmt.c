/*
 * ram_mgmt.c
 *
 *  Created on: 15 jan. 2019
 *      Author: Marcus Persson
 */
#include <stdint.h>
#define ram_addr 0x20000000
char* ram_offset;
int ram_citiroc_write(char *data){
	return 0;
}


void ram_init(uint32_t ram_offset_addr){
	ram_offset = (char*) ram_offset_addr;
}
