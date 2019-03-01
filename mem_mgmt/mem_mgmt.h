/*
 * ram_mgmt.h
 *
 *  Created on: 15 jan. 2019
 *      Author: Marcus Persson
 */

#include "../msp/msp_seqflags.h"

#ifndef MEM_MGMT_MEM_MGMT_H_
#define MEM_MGMT_MEM_MGMT_H_

#define NVM_CITIROC 	0x01u
#define NVM_HVPS 		0x02u
#define NVM_SEQFLAG		0x03u
#define RAM_CITIROC		0x11u
#define RAM_HVPS		0x12u
#define RAM_HISTO		0x13u
#define CITIROC_LEN		128u
#define HVPS_LEN		24u
#define SEQFLAG_LEN		sizeof(msp_seqflags_t)
#define HISTO_LEN		28926

#define ram_addr 0x20000000u
#define nvm_addr 0x60000000u
#define hvps_offset 0x3000u
#define citiroc_offset 0x3008u
#define seqflag_offset 0xF000
#define histo_addr 0x50030000


/* mem_ram_write
 * Function for writing data to RAM.
 * Input: 	Reference to submodule (see defines at top of this file)
 *			uint8_t pointer to where data is to be transferred from
 * Return: None
 */

void mem_ram_write(uint32_t modul, uint8_t *data);


/* mem_read
 * Function for reading data from NVM.
 * Input: 	Reference to submodule (see defines at top of this file)
 *			uint8_t pointer to where data is to be transferred
 * Return: None
 */
void mem_read(uint32_t modul, uint32_t **data);

/* mem_nvm_write
 * Function for writing data to NVM.
 * Input: 	uint32_t Reference to submodule to be written for.
 * 			uint8_t pointer to where data is to be transferred from
 * Return:  int with value 0 if passed, -1 if any failure.
 */

int mem_nvm_write(uint32_t modul, uint8_t* data);

#endif /* MEM_MGMT_MEM_MGMT_H_ */
