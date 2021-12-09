/*
 * CUBES LED exported functions header
 *
 * Copyright © 2020 Theodor Stana, Sonal Shrivastava
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


#ifndef UTILS_LED_H_
#define UTILS_LED_H_


/*
 * LED Blink Type Definitions - based on number of times LED blinks, we can
 * identify the type of error that occurred in MSS.
 */
typedef enum {
	/* Blink 4 times with 500 ms on/off period to indicate MSS power-on reset */
    LED_BLINK_POWER_ON      = 1,
	/*
	 * Blink once for 1 s inside TIM IRQ Handler to indicate completion of DAQ
	 * duration
	 */
    LED_BLINK_DAQ           = 2,
	/* Blink 8 times with 250 ms on/off period to indicate POWER_OFF MSP cmd. */
    LED_BLINK_POWER_OFF     = 3
} led_blink_t;


void led_init(void);
void led_blink(led_blink_t blink_type);
void led_turn_on(void);
void led_turn_off(void);


#endif /* UTILS_LED_H_ */
