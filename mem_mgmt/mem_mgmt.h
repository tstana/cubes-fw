/*
 * ram_mgmt.h
 *
 *  Created on: 15 jan. 2019
 *      Author: Marcus Persson
 */

#ifndef MEM_MGMT_MEM_MGMT_H_
#define MEM_MGMT_MEM_MGMT_H_

/* mem_ram_citiroc_write
 * Function for writing citiroc configuration data to RAM
 * Input: Pointer to data that is to be transferred
 * Return: -1 if any failure, 0 if passed
 */
int mem_ram_citiroc_write(char *data);

/* mem_ram_hvps_write
 * Function for writing hvps configuration data to RAM
 * Input: Pointer to data that is to be transferred
 * Return: -1 if any failure, 0 if passed
 */
int mem_ram_hvps_write(char *data);

/* mem_ram_citiroc_write
 * Function for writing citiroc configuration data to RAM
 * Input: Pointer to data that is to be transferred
 * Return: -1 if any failure, 0 if passed
 */
void mem_ram_hvps_read(char *data);


/* mem_nvm_read
 * Function for reading data from NVM.
 * Input: 	uint32_t pointer to adress in NVM memory
 *			uint8_t pointer to where data is to be transferred
 *			uint32_t length of data to be transferred, fixed amount for each submodule
 * Return: None
 */
void mem_nvm_read(uint32_t *addr, uint8_t *data, uint32_t length);

#endif /* MEM_MGMT_MEM_MGMT_H_ */
