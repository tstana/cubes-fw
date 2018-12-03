/*
 * rtc.h
 *
 *  Created on: 3 dec. 2018
 *      Author: Marcus Persson
 */

#ifndef PERIPHERALS_RTC_RTC_H_
#define PERIPHERALS_RTC_RTC_H_

#include "../../firmware/drivers/mss_rtc/mss_rtc.h"
#include <time.h>

void init_rtc(void);
/*
 * set_time_from_string
 * Takes a string of UNIX time value (seconds since 1 Jan 1970) and converts into mss_calendar_t and sets new time in RTC
 * Input: A string consisting of only UNIX time
 * Output: None
 */
void set_time_from_string(uint8_t time[]);
#endif /* PERIPHERALS_RTC_RTC_H_ */
