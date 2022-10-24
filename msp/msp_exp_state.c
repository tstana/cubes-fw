/**
 * @file      msp_exp_state.c
 * @author    John Wikman
 * @copyright MIT License
 * @brief     The location of the global MSP experiment state.
 *
 * @details
 * The source file where the global variable representing the MSP experiment
 * state is located.
 */

#include "msp_seqflags.h"

#include "msp_exp_state.h"

/* The MSP state */
volatile struct msp_exp_state_information msp_exp_state = {0};

/**
 * @brief Initializes the MSP state.
 * @param seqflags The sequence flags that the MSP state should be initialized
 *                 with.
 *
 * Initializes the MSP state with the specified sequence flags. This function
 * should be called before any MSP transaction takes place. Ideally before
 * starting the I2C driver.
 *
 * If available, the sequence flags passed as an argument should be the
 * sequence flags that were saved to non-volatile memory/storage before the
 * experiment was restarted/powered off.
 */
void msp_exp_state_initialize(msp_seqflags_t seqflags)
{
	msp_exp_state.type = MSP_EXP_STATE_READY;

	msp_exp_state.seqflags = seqflags;

	msp_exp_state.busy = 0;
	msp_exp_state.initialized = 1;
}

/**
 * @brief Returns the sequence flags from the experiment state.
 * @return A copy of the current sequence flags in the experiment state.
 *
 * This function returns a copy of the current sequence flags in the MSP
 * experiment state. These sequence flags should be retreived and saved in
 * non-volatile memory/storage such that they can be used when initializing the
 * experiment state on start up. They should retreived at least once before
 * rebooting and also preferably on regular intervals in case the experiment
 * restarts unexpectedly.
 */
msp_seqflags_t msp_exp_state_get_seqflags(void)
{
	return msp_exp_state.seqflags;
}
