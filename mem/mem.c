/**
 * @file mem.c
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

#include "mem.h"

#include "../msp/msp_exp_state.h"


/*
 * See mem.h for this function's synopsis
 */
int mem_write(uint32_t addr, uint32_t len, uint8_t *data)
{
	if ((addr < MEM_ESRAM_BASE) || (addr >= MEM_ENVM0_BASE))
		return 1;

	uint32_t* memaddr = (uint32_t*)addr;

	for (int i = 0; i < len; i+=4) {
		*memaddr = (data[i+3] << 24) |
		           (data[i+2] << 16) |
		           (data[i+1] <<  8) |
		           (data[i]);
		++memaddr;
	}

	return 0;
}


/*
 * See mem.h for this function's synopsis
 */
void mem_read(uint32_t addr, uint32_t len, uint8_t *data)
{
	uint32_t* memaddr = (uint32_t*)addr;

	for (int i = 0; i < len/4; ++i) {
		data[i] = memaddr[i];
		data[i+1] = memaddr[i] >>  8;
		data[i+2] = memaddr[i] >> 16;
		data[i+3] = memaddr[i] >> 24;
	}
}


/*
 * See mem.h for this function's synopsis
 */
nvm_status_t mem_write_nvm(uint32_t addr, uint32_t len, uint8_t *data)
{
	nvm_status_t status = NVM_unlock(addr, len);

	if((status == NVM_SUCCESS) || (status == NVM_WRITE_THRESHOLD_WARNING))
		status = NVM_write(addr, data, len, NVM_LOCK_PAGE);

	return status;
}


/*
 * See mem.h for this function's synopsis
 */
nvm_status_t mem_reset_counter_increment(void)
{
	uint32_t counter = 0;
	mem_read(MEM_RESET_COUNTER_ADDR, 4, (uint8_t*)&counter);
	counter++;
	return mem_write_nvm(MEM_RESET_COUNTER_ADDR, 4, (uint8_t *)&counter);
}


/*
 * See mem.h for this function's synopsis
 */
uint32_t mem_reset_counter_read(void)
{
	uint32_t counter = 0;
	mem_read(MEM_RESET_COUNTER_ADDR, 4, (uint8_t*)&counter);
	return counter;
}


/*
 * See mem.h for this function's synopsis
 */
nvm_status_t mem_reset_counter_clear(void)
{
	uint8_t resetvalue[4] = {0, 0, 0, 0};
	return mem_write_nvm(MEM_RESET_COUNTER_ADDR, 4, resetvalue);
}


/*
 * See mem.h for this function's synopsis
 */
nvm_status_t mem_save_msp_seqflags(void)
{
	nvm_status_t status;
	msp_seqflags_t s = msp_exp_state_get_seqflags();

	status = NVM_unlock(MEM_SEQFLAGS_ADDR, sizeof(msp_seqflags_t));
	if((status == NVM_SUCCESS) || (status == NVM_WRITE_THRESHOLD_WARNING)) {
		status = NVM_write(MEM_SEQFLAGS_ADDR, (uint8_t*)&s,
		                   sizeof(msp_seqflags_t), NVM_LOCK_PAGE);
	}

	return status;
}


/*
 * See mem.h for this function's synopsis
 */
void mem_restore_msp_seqflags(void)
{
	msp_seqflags_t s;
	mem_read(MEM_SEQFLAGS_ADDR, MEM_SEQFLAGS_LEN, (uint8_t*)&s);
	msp_exp_state_initialize(s);
}
