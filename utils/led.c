/*
 * CUBES LED functions
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


#include <stdint.h>

#include "mss_gpio/mss_gpio.h"
#include "led.h"
#include "timer_delay.h"
#include "CMSIS/system_m2sxxx.h"


static uint32_t g_gpio_pattern;


/**
 * @brief Initialization function for LED at GPIO0 called upon MSS power on reset
 *
 * This function is called is called in main() function at the start to configure
 * the GPIO0 in OUTPUT_MODE such that this GPIO0 configuration need not be
 * changed afterwards before calling led_blink().
 *
 */
void led_init(void)
{
	/* Initialize MSS GPIOs */
	MSS_GPIO_init();

	/* Configure MSS GPIO0 in OUTPUT_MODE */
	MSS_GPIO_config( MSS_GPIO_0 , MSS_GPIO_OUTPUT_MODE);

	g_gpio_pattern = MSS_GPIO_get_outputs();

	g_gpio_pattern &= ~(MSS_GPIO_0_MASK);

	MSS_GPIO_set_outputs(g_gpio_pattern);
}


/**
 * @brief Blink LED at GPIO0 for a certain number of milliseconds
 *
 * This function is called to make LED at GPIO0 (configured as output) blink
 * once for a limited amount of milliseconds, specified by parameter.
 *
 * @b NOTE:
 *
 * - Should be called after led_init() function since it configures
 * GPIO0 in OUTPUT_MODE upon MSS power on reset.
 *
 * @param ms_delay[in]   Blink delay in ms_delay
 *
 */
void led_blink_for(uint32_t ms_delay)
{
	led_turn_on();
	timer_delay(ms_delay);
	led_turn_off();
}


/**
 * @brief Blink LED at GPIO0 a number of times with delay in between
 *
 * This function is called to make LED at GPIO0 (configured as output) blink
 * a number of times.
 *
 * @b NOTE:
 *
 * - Should be called after led_init() function since it configures
 * GPIO0 in OUTPUT_MODE upon MSS power on reset.
 *
 * @param num[in]
 * @param ms_delay[in]   Blink delay in ms_delay
 *
 */void led_blink_repeat(uint32_t num, uint32_t ms_delay)
{
	int i;
	for (i = 0; i < num; ++i) {
		led_blink_for(ms_delay);
		timer_delay(ms_delay);
	}
}


/**
 * @brief Turn on LED at GPIO0.
 *
 * @b NOTE:
 *
 * - Call this function only after led_init() function since it configures
 * GPIO0 in OUTPUT_MODE upon MSS power on reset.
 *
 */
void led_turn_on(void)
{
	g_gpio_pattern = MSS_GPIO_get_outputs();
	g_gpio_pattern |= MSS_GPIO_0_MASK;
	MSS_GPIO_set_outputs(g_gpio_pattern);
}



/**
 * @brief Turn off LED at GPIO0.
 *
 * @b NOTE:
 *
 * - Call this function only after led_init() function since it configures
 * GPIO0 in OUTPUT_MODE upon MSS power on reset.
 *
 */
void led_turn_off(void)
{
	g_gpio_pattern = MSS_GPIO_get_outputs();
	g_gpio_pattern &= ~(MSS_GPIO_0_MASK);
	MSS_GPIO_set_outputs(g_gpio_pattern);
}
