/*
 * timer_delay.c
 *
 *  Created on: 19-Feb-2021
 *      Author: Sonal Shrivastava
 */

#include <stdint.h>

#include "drivers/mss_timer/mss_timer.h"
#include "CMSIS/system_m2sxxx.h"
#include "timer_delay.h"
#include "led.h"


volatile uint32_t delay_counter;


void SysTick_Handler(void)
{
    delay_counter++;
}


void timer_delay_init(void)
{
    // Set reload register to generate an interrupt every millisecond
    SysTick->LOAD = (uint32_t)((SystemCoreClock / 1000) - 1);

    // Reset the SysTick counter value
    SysTick->VAL = 0UL;

    // Set SysTick source and IRQ
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk);
}




void timer_delay(uint32_t ms)
{
    // Enable the SysTick timer
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    // Wait for a specified number of milliseconds
    delay_counter = 0;
    while (delay_counter < ms);

    // Disable the SysTick timer
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}
