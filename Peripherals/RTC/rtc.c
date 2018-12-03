/*
 * rtc.c
 *
 *  Created on: 3 dec. 2018
 *      Author: Marcus Persson
 */

#include "rtc.h"
#include <stdlib.h>
void init_rtc(void){
	MSS_RTC_init(MSS_RTC_CALENDAR_MODE, 32767);
}

void set_time_from_string(uint8_t time[]){
	MSS_RTC_stop();
	mss_rtc_calendar_t set_time;
	time_t sec_time;
	int num_time=0;
	struct tm *cal_time;
	num_time = atoi(time);
	sec_time = num_time;
	cal_time=gmtime(&sec_time);
	set_time.second=cal_time->tm_sec;
	set_time.minute=cal_time->tm_min;
	set_time.hour=cal_time->tm_hour;
	set_time.day=cal_time->tm_mday;
	set_time.month=cal_time->tm_mon+1;
	set_time.year=cal_time->tm_year;
	set_time.weekday=cal_time->tm_wday+1;
	MSS_RTC_set_calendar_count(&set_time);
	MSS_RTC_start();
}
