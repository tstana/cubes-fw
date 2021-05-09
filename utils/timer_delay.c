/*
 * CUBES delay timer functions
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

#include "drivers/mss_timer/mss_timer.h"
#include "CMSIS/system_m2sxxx.h"
#include "timer_delay.h"


static volatile uint32_t delay_counter;


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
