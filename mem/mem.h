/**
 * @file mem.h
 *
 *  Created on: 15 jan. 2019
 *  Copyright © 2022 Theodor Stana (based on previous code by Marcus Persson)
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

#ifndef _MEM_H_
#define _MEM_H_

#include "../firmware/cubes_hw_platform.h"
#include "../firmware/drivers/mss_nvm/mss_nvm.h"


/* Define eSRAM base and top addresses (seemingly missing in CMSIS...?) */
#define MEM_ESRAM_BASE    (0x20000000)
#define MEM_ESRAM_TOP     (0x3fffffff)

/* Configuration RAM contents */
#define MEM_CITIROC_CONF_ADDR   (CFG_RAM + 0x00)
#define MEM_CITIROC_CONF_LEN    (144u)
#define MEM_CITIROC_PROBE_ADDR  (CFG_RAM + 0x90)
#define MEM_CITIROC_PROBE_LEN   ( 32u)


/*
 * Histo-RAM contents in gateware:
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
#define MEM_HISTO_LEN_GW        (24832u)
#define MEM_HISTO_NUM_BINS_GW   ( 2048u)
#define MEM_HISTO_HDR_LEN       (  256u)


/* Define eNVM base and top addresses (seemingly missing in CMSIS...?) */
#define MEM_ENVM0_BASE    (0x60000000)
#define MEM_ENVM0_TOP     (0x60003fff)

/* Reset Counter */
#define MEM_RESET_COUNTER_ADDR  (0x6000f000)
#define MEM_RESET_COUNTER_LEN   (4u)

/* MSP Sequence Flags */
#define MEM_SEQFLAGS_ADDR       (0x6000f100)
#define MEM_SEQFLAGS_LEN        sizeof(msp_seqflags_t)

#define MEM_CLEAN_POWEROFF_ADDR (0x6000f200)

/* Citiroc configuration in Non-Volatile Memory */
#define MEM_CITIROC_CONF_ADDR_NVM   (0x60010000)
#define MEM_CITIROC_CONF_ID_ADDR    (0x6001fff0)
#define MEM_CITIROC_CONF_ID_LEN     (1u)


/**
 * @brief Write data to ESRAM memory address
 *
 * @param   addr  Memory address to write to
 * @param   len   Number of bytes to write to memory
 * @param   data  Pointer to where data is to be read from
 * @return  0  if the write was successful
 *          1  if the write was not within ESRAM or peripheral memory space
 *             (0x20000000 to 0x5fffffff)
 */
int mem_write(uint32_t addr, uint32_t len, uint8_t *data);


/**
 * @brief Read data from memory
 *
 * @param   addr  Reference to submodule (see defines at top of this file)
 * @param   len   Number of bytes to read from memory
 * @param   data  Pointer to where data is to be written to
 */
void mem_read(uint32_t addr, uint32_t len, uint8_t *data);

/**
 * @brief Write data to MSS Non-Volatile Memory
 *
 * This function unlocks `len` memory addresses starting from `addr` and
 * attempts to write them using the `mss_nvm` API.
 *
 * @param addr  Address to start writing from
 * @param len   Number of bytes to write
 * @param data  Pointer to data to write to NVM
 * @return `NVM_SUCCESS` in case of successful write
 *         `NVM_WRITE_THRESHOLD_WARNING` in case the number of writes threshold
 *                                       to a memory address has been reached.
 *                                       Note that the write could still have
 *                                       been performed successfully, but this
 *                                       is not guaranteed.
 *         In case of other error: See the `NVM_unlock()` and `NVM_write()`
 *         functions of `mss_nvm.h`.
 */
nvm_status_t mem_write_nvm(uint32_t addr, uint32_t len, uint8_t *data);


/**
 * @brief Read, increment, then write reset counter at appropriate NVM address
 *
 * @return See return status of `NVM_write()` function in `mss_nvm.h`.
 */
nvm_status_t mem_reset_counter_increment(void);


/**
 * @brief Read reset counter from NVM.
 * @return Reset counter value
 */

uint32_t mem_reset_counter_read(void);


/**
 * @brief Clear reset counter value stored in NVM (reset its contents to '0')
 *
 * @return See return status of `NVM_write()` function in `mss_nvm.h`.
 */
nvm_status_t mem_reset_counter_clear(void);


/**
 * @brief Save MSP sequence flags to NVM
 * @return `NVM_SUCCESS` in case of successful write
 *         `NVM_WRITE_THRESHOLD_WARNING` in case the number of writes threshold
 *                                       to a memory address has been reached.
 *                                       Note that the write could still have
 *                                       been performed successfully, but this
 *                                       is not guaranteed.
 *         In case of other error: See the `NVM_unlock()` and `NVM_write()`
 *         functions of `mss_nvm.h`.
 */
nvm_status_t mem_save_msp_seqflags(void);


/**
 * @brief Read MSP sequence flags from NVM and restore them for a new transaction
 */
void mem_restore_msp_seqflags(void);


#endif /* _MEM_MGMT_H_ */
