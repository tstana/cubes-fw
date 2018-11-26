/*
 * msp_exp_state.h
 * Author: John Wikman
 *
 * Defines the internal state of MSP.
 */

#ifndef MSP_EXP_STATE_H
#define MSP_EXP_STATE_H

#include "../msp/msp_seqflags.h"

typedef enum {
	MSP_EXP_STATE_READY,
	MSP_EXP_STATE_OBC_SEND_TX,
	MSP_EXP_STATE_OBC_SEND_TX_DUPLICATE, /* Receiving duplicate transaction */
	MSP_EXP_STATE_OBC_REQ_RESPONSE,
	MSP_EXP_STATE_OBC_REQ_TX
} msp_exp_state_type_t;

struct msp_exp_state_information {
	/* Keeps track of the type of state we are in */
	msp_exp_state_type_t type;

	/* Keeps track of whether the state is initialized or not */
	unsigned char initialized;

	/*
	 * Keeps track of if we are currently processing something or if we are
	 * ready for the next frame.
	 */
	unsigned char busy;

	/*
	 * Keep track of whether we have called the corresponding transaction start
	 * function in msp_exp_handler.h
	 */
	unsigned char has_called_start;

	/* Current transaction ID, frame-ID (for OBC Request), and prev frame-ID (for OBC Send) */ 
	unsigned char transaction_id;
	unsigned char frame_id;
	unsigned char last_received_frame_id;

	/* OP code of the transaction */
	unsigned char opcode;

	/*
	 * The sequence flags of MSP. The flag for a given OP code is set to the
	 * transaction ID of the last successful transaction with that OP code.
	 */
	msp_seqflags_t seqflags;

	/* Number of bytes to be sent or received in a transaction. */
	unsigned long total_length;
	/* Number of bytes that has been sent or received so far. */
	unsigned long processed_length;
	/* The number of data bytes that was sent with the last frame */
	unsigned long prev_data_length;
};


/* Declare existance of the MSP state */
extern volatile struct msp_exp_state_information msp_exp_state;

/*
 * Initializes the MSP state with the specified sequence flags.
 */
void msp_exp_state_initialize(msp_seqflags_t seqflags);

/*
 * Returns the sequence flags from the experiment state.
 */
msp_seqflags_t msp_exp_state_get_seqflags(void);

#endif
