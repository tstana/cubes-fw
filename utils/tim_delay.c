/*
 * tim_delay.c
 *
 *  Created on: 19-Feb-2021
 *      Author: Sonal Shrivastava
 */

#include <stdint.h>

#include "drivers/mss_timer/mss_timer.h"
#include "CMSIS/system_m2sxxx.h"
#include "tim_delay.h"
#include "led.h"


// Delay counter
//static volatile uint32_t DelayCounter;
volatile uint32_t DelayCounter;


void SysTick_Handler(void)
{
    DelayCounter++;
}


void tim_init(void)
{
    // Set reload register to generate an interrupt every millisecond.
    SysTick->LOAD = (uint32_t)((SystemCoreClock / 1000) - 1);

    // Set priority for SysTick IRQ
    //NVIC_SetPriority(SysTick_IRQn,(1 << __NVIC_PRIO_BITS) - 1);

    // Reset the SysTick counter value.
    SysTick->VAL = 0UL;

    // Set SysTick source and IRQ.
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk);
}




void delay(uint32_t ms)
{
    // Enable the SysTick timer
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    // Wait for a specified number of milliseconds
    DelayCounter = 0;
    while (DelayCounter < ms);

    // Disable the SysTick timer
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


void delay_test(void)
{
    tim_init();
    led_turn_off();
    delay(500);
    led_turn_on();
}
