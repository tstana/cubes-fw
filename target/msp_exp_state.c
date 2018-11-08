/*
 * msp_exp_state.c
 * Author: John Wikman
 *
 * The source file where the global variable representing the MSP state is
 * located.
 */

#include "msp_seqflags.h"

#include "msp_exp_state.h"

/* The MSP state */
volatile msp_exp_state_t msp_exp_state = {0};

/*
 * Initializes the MSP state with the specified sequence flags.
 *
 * Arguments
 *  seqflags: The sequence flags that the MSP state will have.
 */
void msp_exp_state_initialize(msp_seqflags_t seqflags)
{
	msp_exp_state.type = MSP_EXP_STATE_READY;

	msp_exp_state.seqflags = seqflags;

	msp_exp_state.busy = 0;
	msp_exp_state.initialized = 1;
}
