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


enum hvps_cmd_counter {
	HVPS_CMDS_SENT = 1,
	HVPS_CMDS_ACKED,      // Received proper reply from MPPC bias module
	HVPS_CMDS_FAILED      // Received "hxx" reply from MPPC bias module
};


void      hvps_init(void);
int       hvps_set_temp_corr_factor(uint8_t* command);
int       hvps_set_temporary_voltage(uint16_t v);
void      hvps_send_cmd(char *cmd);
uint8_t   hvps_is_on(void);
uint16_t  hvps_get_latest_temp(void);
uint16_t  hvps_get_latest_volt(void);
uint16_t  hvps_get_latest_curr(void);
uint16_t  hvps_get_cmd_counter(enum hvps_cmd_counter);


#endif /* _HVPS_C11204_02_H_ */
