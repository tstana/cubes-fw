/*
 * led.c
 *
 *  Created on: 29-Jan-2021
 *      Author: Sonal Shrivastava
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "drivers/mss_timer/mss_timer.h"
#include "drivers_config/sys_config/sys_config_mss_clocks.h"
#include "mss_gpio/mss_gpio.h"
#include "led_error.h"
#include "led.h"
#include "CMSIS/system_m2sxxx.h"


/*
 * LEDs masks used to switch on/off LED through GPIOs.
 */
#define LEDS_MASK   (uint32_t)0xAAAAAAAA


volatile uint32_t g_gpio_pattern = LEDS_MASK;
static uint8_t num_of_blinks, count;
static uint32_t timer2_load_value = MSS_SYS_APB_0_CLK_FREQ;

/* Local function prototypes */
void led_set_high(void);
void led_conf_tim2_periodic(void);


/**
 * @brief Blink function for LED at GPIO0 called upon MSS power on reset
 *
 * This function is called is called in main() function at the start to indicate power on
 * reset of MSS. It shall blink @b LED_RESET number of times (defined in led_error.h).
 * It configures the GPIO0 in OUTPUT_MODE such that this GPIO0 configuration need not be
 * changed afterwards before calling led_custom_blink().
 *
 */
void led_blink_reset(void)
{
    /* Initialize MSS GPIOs */
    MSS_GPIO_init();

    /* Configure MSS GPIO0 */
    MSS_GPIO_config( MSS_GPIO_0 , MSS_GPIO_OUTPUT_MODE);

    MSS_GPIO_set_outputs(g_gpio_pattern);

    num_of_blinks = LED_RESET;

    /* Configure TIM2 in periodic mode*/
    led_conf_tim2_periodic();
}


/**
 * @brief Configure TIM2 in periodic mode
 *
 * @b NOTE:
 *
 * - This function passes a pre-defined delay value to be loaded into the timer
 * which is equal to 500ms.
 *
 */
void led_conf_tim2_periodic(void)
{
    /*
     * Configure Timer2 - Use the timer input frequency(100MHz) div by 2 as load value to achieve half a second
     * periodic interrupt.
     */
    MSS_TIM2_init(MSS_TIMER_PERIODIC_MODE);

    /* Configure the timer period to 500ms*/
    MSS_TIM2_load_background(timer2_load_value/2);
    MSS_TIM2_start();
    MSS_TIM2_enable_irq();
}




/**
 * @brief Interrupt handler for Timer2
 *
 */
void Timer2_IRQHandler(void)
{
    count++;

    /* Toggle GPIO0 output pattern without affecting GPIO ports outputs */
    g_gpio_pattern = MSS_GPIO_get_outputs();

    /* Toggle GPIO0 output pattern by doing an exclusive OR */
    g_gpio_pattern ^= 0x00000001u;
    MSS_GPIO_set_outputs(g_gpio_pattern);

    if(count%2 == 0)
    {
        num_of_blinks--;
    }

    if(num_of_blinks == 0)
    {
        MSS_TIM2_disable_irq();
        MSS_TIM2_stop();

        /* Set LED high to indicate board's operational status hereafter */
        led_set_high();
    }

    /* Clear TIM2 interrupt */
    MSS_TIM2_clear_irq();
}


/**
 * @brief Set LED at GPIO0 high.
 *
 * @b NOTE:
 *
 * - Call this function only after led_blink_reset() function since it configures
 * GPIO0 in OUTPUT_MODE upon MSS power on reset.
 *
 */
void led_set_high(void)
{
    g_gpio_pattern = MSS_GPIO_get_outputs();

    /* Toggle GPIO0 output pattern by doing an exclusive OR */
    g_gpio_pattern |= 0x00000001u;
    MSS_GPIO_set_outputs(g_gpio_pattern);
}


/**
 * @brief Blink function for LED at GPIO0
 *
 * This function is called to make LED at GPIO0 (configured as output) blink
 * a certain number of times defined by the enumeration #led_error_type_t present in
 * led_error.h file.
 *
 * @b NOTE:
 *
 * - Should be called after led_blink_reset() function since it configures
 * GPIO0 in OUTPUT_MODE upon MSS power on reset.
 *
 * @param blinks[in]      Number of blinks for LED
 *
 */
void led_custom_blink(led_error_type_t blinks)
{
    num_of_blinks = blinks;

    /* Configure TIM2 in periodic mode*/
    led_conf_tim2_periodic();
}


