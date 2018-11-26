/*
 * FPGA_SYSINIT.c
 *
 *  Created on: 9 nov. 2018
 *      Author: Marcus Persson
 */
#include <stdint.h>
#include "../firmware/CMSIS/m2sxxx.h"

void init_Sleep(void){
	SCB->SCR = ((uint32_t) 0 << SCB_SCR_SEVONPEND_Pos) & SCB_SCR_SEVONPEND_Msk;
	//SCB->SCR->SEVONPEND = 0; /* only enabled interrupts or events can wake up processor */
	SCB->SCR = ((uint32_t) 0 << SCB_SCR_SLEEPDEEP_Pos) & SCB_SCR_SLEEPDEEP_Msk;
	//SCB->SCR->SLEEPDEEP = 0; /* 0u normal sleep, 1u deep sleep mode */
	SCB->SCR = ((uint32_t)0 << SCB_SCR_SLEEPONEXIT_Pos) & SCB_SCR_SLEEPONEXIT_Msk;
	//SCB->SCR->SLEEPONEXIT = 0; /* Do not go back to sleep when ISR is done */

}

void init_Sysreg(void){
	init_Sleep();
}

