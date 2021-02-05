/*
 * led_error.h
 *
 *  Created on: 27-Jan-2021
 *      Author: Sonal Shrivastava
 */

#ifndef FIRMWARE_DRIVERS_MSS_GPIO0_LED_MSS_GPIO0_LED_ERROR_H_
#define FIRMWARE_DRIVERS_MSS_GPIO0_LED_MSS_GPIO0_LED_ERROR_H_

/* LED Error Type Definitions - based on number of times LED blinks, we can identify type of error occurred in MSS*/
typedef enum {
    LED_RESET = 4,       // Blink 4 times to indicate MSS power-on reset
    LED_OTHER_ERROR      // Blink 5 times for another type of error
} led_error_type_t;


#endif /* FIRMWARE_DRIVERS_MSS_GPIO0_LED_MSS_GPIO0_LED_ERROR_H_ */
