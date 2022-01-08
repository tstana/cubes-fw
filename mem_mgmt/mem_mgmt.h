/**
 * @file mem_mgmt.h
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

#include "../firmware/drivers/mss_nvm/mss_nvm.h"
#include "../firmware/cubes_hw_platform.h"


#ifndef _MEM_MGMT_H_
#define _MEM_MGMT_H_

#define NVM_CITIROC     		(0x01u)
#define NVM_HVPS        		(0x02u)
#define NVM_SEQFLAG     		(0x03u)
#define NVM_RESET       		(0x04u)
#define NVM_CITIROC_CONF_NUM	(0x05u)
#define RAM_CITI_CONF   		(0x11u)
#define RAM_CITI_PROBE  		(0x14u)

#define CITIROC_OFS     (0x00)
#define CITIROC_LEN     (144u)
#define CITIROC_NUM_LEN	(4u)
#define PROBE_OFS       (0x90)
#define PROBE_LEN       (32u)

#define SEQFLAG_LEN     sizeof(msp_seqflags_t)

#define RAM_ADDR        		(0x20000000u)
#define NVM_ADDR        		(0x60000000u)
#define SEQFLAG_OFFSET  		(0xF100)
#define CITIROC_CONF_NUM_OFFSET	(0xFFF0)
#define CITIROC_OFFSET  		(0x10000)

#define SEQ_FLAG_SAVE_INTERVAL (16u)

/*
 * Histogram Organization
 *
 *  Histo-RAM Header     :   256
 *  Histo-RAM HG, Ch.  0 :  4096 (2048 x 2-byte bins)
 *  Histo-RAM LG, Ch.  0 :  4096
 *  Histo-RAM HG, Ch. 16 :  4096
 *  Histo-RAM LG, Ch. 16 :  4096
 *  Histo-RAM HG, Ch. 31 :  4096
 *  Histo-RAM LG, Ch. 31 :  4096
 *                 ------------
 *                 Total : 24832 bytes
 */
#define HISTO_ADDR      (HISTO_RAM)
	#define HISTO_LEN            (24832)
	#define HISTO_NUM_BINS_GW    ( 2048)
	#define HISTO_HDR_NUM_BYTES  (  256)

#define NVM_SEQFLAG_ADDR  (0x6000F000)


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


/**
 * @brief Save MSP sequence flags to NVM
 * @return Status of NVM unlock or write operation, search for "nvm_status_t" in
 *         mss_nvm.h
 */
nvm_status_t nvm_save_msp_seqflags(void);

void nvm_restore_msp_seqflags(void);

#endif /* _MEM_MGMT_H_ */
