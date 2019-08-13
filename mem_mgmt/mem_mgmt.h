/**
 * ram_mgmt.h
 *
 *  Created on: 15 jan. 2019
 *  @author Marcus Persson
 */

#include "../msp/msp_seqflags.h"
#include "../firmware/cubes_hw_platform.h"

#ifndef MEM_MGMT_MEM_MGMT_H_
#define MEM_MGMT_MEM_MGMT_H_

#define NVM_CITIROC 	0x01u
#define NVM_HVPS 		0x02u
#define NVM_SEQFLAG		0x03u
#define NVM_RESET		0x04u
#define RAM_CITI_CONF	0x11u
#define RAM_HVPS		0x12u
#define RAM_HISTO		0x13u
#define RAM_CITI_PROBE	0x14u

#define HVPS_OFS		0x00
#define HVPS_LEN		12u
#define CITIROC_OFS		0x10
#define CITIROC_LEN		144u
#define PROBE_OFS		0xA0
#define PROBE_LEN		32u

#define SEQFLAG_LEN		sizeof(msp_seqflags_t)
#define HISTO_LEN		24832

#define RAM_ADDR		0x20000000u
#define NVM_ADDR		0x60000000u
#define HVPS_OFFSET 	0xF000u
#define CITIROC_OFFSET 	0xF010u
#define SEQFLAG_OFFSET 	0xF100
#define HISTO_ADDR 		0x50030000


/** mem_ram_write
 * Function for writing data to RAM.
 * @param 	Reference to submodule (see defines at top of this file)
 * @param	uint8_t pointer to where data is to be transferred from
 * Return: None
 */

void mem_ram_write(uint32_t modul, uint8_t *data);


/** mem_read
 * Function for reading data from NVM.
 * @param 	Reference to submodule (see defines at top of this file)
 * @param	uint8_t pointer to where data is to be transferred
 * Return: None
 */
uint32_t mem_read(uint32_t modul, uint32_t *data);

/** mem_nvm_write
 * Function for writing data to NVM.
 * @param 	uint32_t Reference to submodule to be written for.
 * @param	uint8_t pointer to where data is to be transferred from
 * @return  int with value 0 if passed, -1 if any failure.
 */

int mem_nvm_write(uint32_t modul, uint8_t* data);


/**
 *  nvm_reset_counter_increment
 * @brief nvm reset counter which increments by 1 every time it's called
 * 		then saves to nvm location.
 */
void nvm_reset_counter_increment(void);

/**
 * nvm_reset_counter_read
 * @brief Read function of NVM reset counter.
 * @return uint32_t Value of counter
 */

uint32_t nvm_reset_counter_read(void);

/**
 * nvm_reset_counter_reset
 * @brief Reset value for NVM reset counter
 */
void nvm_reset_counter_reset(void);

#endif /* MEM_MGMT_MEM_MGMT_H_ */
