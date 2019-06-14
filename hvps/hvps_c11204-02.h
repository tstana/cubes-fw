/*
 * hvps_c11204-02.h
 *
 *  Created on: 10 jan. 2019
 *      Author: Marcus Persson
 */

#ifndef HM_HVPS_H_
#define HM_HVPS_H_

/*
 * Initalizes all interrupts and timestamp timer
 * Input: Memory adress of Housekeeping data
 * Output: None
 *
 */
void hvps_init(uint32_t memory);

/*
 * hvps_send_voltage()
 * Takes incoming voltage command and transforms it into hex and formats the HST command to uart.
 * Input: A string literal with voltage in decimal form
 * Output: Return -1 for fail, 0 for pass.
 */

int hvps_set_voltage(uint8_t* command);

/*
 * hvps_turn_on()
 * Command for turning on the hvps
 *
 */
void hvps_turn_on(void);

/*
 * hvps_turn_off()
 * Command for turning the hvps off
 */

void hvps_turn_off(void);

void hvps_get_voltage(void);

void hvps_get_current(void);

void hvps_get_temp(void);

#endif /* HM_HVPS_H_ */
