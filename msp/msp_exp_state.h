/**
 * @file      msp_exp_state.h
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Defines the structure of the internal MSP experiment state.
 *
 * Defines the struct msp_exp_state_information which keeps track of the
 * internal MSP state of the experiment.
 */

#ifndef MSP_EXP_STATE_H
#define MSP_EXP_STATE_H

#include "msp_seqflags.h"

typedef enum {
	MSP_EXP_STATE_READY, /**< Ready to start a new transaction. */
	MSP_EXP_STATE_OBC_SEND_RX, /**< In an OBC Send transaction. */
	MSP_EXP_STATE_OBC_SEND_RX_DUPLICATE, /**< Receiving a duplicate OBC Send transaction. */
	MSP_EXP_STATE_OBC_REQ_RESPONSE, /**< Responding to an OBC Request transaction. */
	MSP_EXP_STATE_OBC_REQ_TX /**< In an OBC Request transaction. */
} msp_exp_state_type_t;

/**
 * @brief Contains fields necessary to keep track of the MSP state for an
 *        experiment.
 */
struct msp_exp_state_information {
	/** 
	 * @brief The state type that the experiment is in.
	 */
	msp_exp_state_type_t type;

	/**
	 * @brief A boolean value to keep track of whether the state is initialized
	 *        or not.
	 */
	unsigned char initialized;

	/**
	 * @brief A boolean value to keep track of if the experiment is busy with
	 *        processing an MSP frame.
	 *
	 * This should be used to make sure that two MSP frames are not processed
	 * at the same time.
	 */
	unsigned char busy;

	/**
	 * @brief A boolean value to keep track of if an transaction start function
	 *        from msp_exp_handler.h has been called in the current transaction.
	 */
	unsigned char has_called_start;

	/**
	 * @brief The transaction-ID of the current transaction.
	 */
	unsigned char transaction_id;

	/**
	 * @brief The frame-ID of the frame last sent by the experiment in an OBC
	 *        Request transaction.
	 */
	unsigned char frame_id;

	/**
	 * @brief The frame-ID of the frame last received from the OBC in an OBC
	 *        Send transaction.
	 */ 
	unsigned char last_received_frame_id;

	/**
	 * @brief The opcode of the ongoing transaction.
	 */
	unsigned char opcode;

	/**
	 * @brief The sequence flags of the experiment state.
	 *
	 * The sequence flags keeps track of the transaction-ID of the last
	 * successful transaction for each opcode.
	 */
	msp_seqflags_t seqflags;


	/** 
	 * @brief The total number of bytes to be sent or received in the current
	 *        transaction.
	 */
	unsigned long total_length;

	/**
	 * @brief The number of bytes that has been sent or received so far in the
	 *        current transaction.
	 */
	unsigned long processed_length;

	/**
	 * @brief The number of bytes in the data field of the last sent data frame
	 *        in an OBC Request transaction.
	 */
	unsigned long prev_data_length;
};


/**
 * Declares the existance of the MSP experiment state.
 */
extern volatile struct msp_exp_state_information msp_exp_state;

void msp_exp_state_initialize(msp_seqflags_t seqflags);
msp_seqflags_t msp_exp_state_get_seqflags(void);

#endif /* MSP_EXP_STATE_H */
