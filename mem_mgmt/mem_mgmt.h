/*
 * ram_mgmt.h
 *
 *  Created on: 15 jan. 2019
 *      Author: Marcus Persson
 */

#ifndef MEM_MGMT_MEM_MGMT_H_
#define MEM_MGMT_MEM_MGMT_H_

int mem_ram_citiroc_write(char *data);

int mem_ram_hvps_write(char *data);

void mem_ram_hvps_read(char *data);

void mem_nvm_read(uint32_t *addr, uint8_t *data, uint32_t length);

#endif /* MEM_MGMT_MEM_MGMT_H_ */
