/*
 * led.h
 *
 *  Created on: 29-Jan-2021
 *      Author: Sonal Shrivastava
 */

#ifndef UTILS_LED_H_
#define UTILS_LED_H_


/* LED Blink Type Definitions - based on number of times LED blinks, we can identify type of error occurred in MSS*/
typedef enum {
    LED_BLINK_DAQ = 1,
    LED_BLINK_RESET = 4,       // Blink 4 times to indicate MSS power-on reset
    LED_BLINK_OTHER_ERROR      // Blink 5 times for another type of error
} led_blink_t;


void led_init(void);
void led_custom_blink(led_blink_t blinks);


#endif /* UTILS_LED_H_ */
