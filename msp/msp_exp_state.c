/*
 * msp_exp_state.c
 * Author: John Wikman
 *
 * The source file where the global variable representing the MSP state is
 * located.
 */

#include "../msp/msp_exp_state.h"

#include "../msp/msp_seqflags.h"

/* The MSP state */
volatile struct msp_exp_state_information msp_exp_state = {0};

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

/*
 * Returns the sequence flags from the experiment state.
 */
msp_seqflags_t msp_exp_state_get_seqflags(void)
{
	return msp_exp_state.seqflags;
}
