/*
 * Source file for communicating to Hamamatsu C11204-02 MPPC bias module
 *
 *  Created on: 5 Nov. 2020
 *
 * Copyright © 2020 Theodor Stana (based on old code by Marcus Persson)
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

#ifndef _HVPS_C11204_02_H_
#define _HVPS_C11204_02_H_

/* Type Definitions */
enum hvps_cmd_counter {
	HVPS_CMDS_SENT = 1,
	HVPS_CMDS_ACKED,      // Received proper reply from MPPC bias module
	HVPS_CMDS_FAILED      // Received "hxx" reply from MPPC bias module
};

struct hvps_temp_corr_factor {
	uint16_t dtp1;
	uint16_t dtp2;
	uint16_t dt1;
	uint16_t dt2;
	uint16_t vb;
	uint16_t tb;
};


/* Function Definitions */
void      hvps_init(void);
int       hvps_turn_on();
int       hvps_turn_off();
int       hvps_reset();
int       hvps_set_temp_corr_factor(struct hvps_temp_corr_factor *f);
int       hvps_temp_compens_en();
int       hvsp_temp_compens_dis();
int       hvps_set_temporary_voltage(uint16_t vb);
int       hvps_is_on(void);
uint16_t  hvps_get_temp(void);
uint16_t  hvps_get_voltage(void);
uint16_t  hvps_get_current(void);
uint16_t  hvps_get_status(void);
uint16_t  hvps_get_cmd_counter(enum hvps_cmd_counter);
uint16_t  hvps_get_last_cmd_err(void);


#endif /* _HVPS_C11204_02_H_ */
