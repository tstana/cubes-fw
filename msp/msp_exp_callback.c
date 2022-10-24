/**
 * @file      msp_exp_callback.c
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Implements the callback functions for the experiment side of MSP.
 *
 * @details
 * Implements the MSP Callbacks defined in msp_exp_callback.h.
 */

#include "msp_debug.h"
#include "msp_endian.h"
#include "msp_opcodes.h"

#include "msp_exp_callback.h"
#include "msp_exp_definitions.h"
#include "msp_exp_error.h"
#include "msp_exp_frame.h"
#include "msp_exp_handler.h"
#include "msp_exp_state.h"


static int handle_incoming_frame(const unsigned char *frame, unsigned long len);
static int handle_incoming_data_frame(const unsigned char *data, unsigned char frame_id, unsigned long len);
static int handle_incoming_header_frame(unsigned char opcode, unsigned char frame_id, unsigned long dl);
static int handle_incoming_control_frame(unsigned char opcode, unsigned char frame_id);
static int handle_incoming_system_frame(unsigned char opcode, unsigned char frame_id);
static int handle_incoming_request_frame(unsigned char opcode);
static int handle_incoming_send_frame(unsigned char opcode, unsigned char frame_id, unsigned long dl);

static int handle_outgoing_frame(unsigned char *buf, unsigned long *len);
static int handle_outgoing_response_frame(unsigned char *buf, unsigned long *len);
static int handle_outgoing_data_frame(unsigned char *buf, unsigned long *len);
static int handle_outgoing_acknowledge_frame(unsigned char *buf, unsigned long *len);

static void ensure_ready_state(void);

/*
 * Implementation of the MSP receive callback function. This function just
 * performs a sanity check on the MSP state to make sure that it is safe to
 * proceed, then distributes the actual functionality to other functions.
 *
 * Arguments
 *  data: Pointer to the received data from the OBC. 
 *  len: Number of bytes received from the OBC.
 */
int msp_recv_callback(const unsigned char *data, unsigned long len)
{
	int code;

	if (!msp_exp_state.initialized) /* Check that we are initialized */
		msp_exp_state_initialize(msp_seqflags_init());
	else if (msp_exp_state.busy) /* If we are busy, just return */
		return MSP_EXP_ERR_IS_BUSY;

	/* Ignore the frame is the FCS is invalid. (from_obc = 1) */
	if (!msp_exp_frame_fcs_valid(data, 1, len))
		return MSP_EXP_ERR_FCS_MISMATCH;

	/* Now mark the MSP state as busy and carry on */
	msp_exp_state.busy = 1;
	code = handle_incoming_frame(data, len);
	msp_exp_state.busy = 0;

	return code;
}

/*
 * Implementation of the MSP send callback function. This function just
 * performs a sanity check on the MSP state to make sure that it is safe to
 * proceed, then distributes the actual functionality to other functions.
 *
 * Arguments
 *  data: Pointer to a buffer where the data to be sent will be stored. The
 *        buffer must be at least MSP_MTU+5 in size. (Make sure to set MSP_MTU
 *        in msp_exp_definitions.h)
 *  len: A pointer to a 32-bit unsigned int that represents the number of bytes
 *       to be sent.
 */
int msp_send_callback(unsigned char *data, unsigned long *len)
{
	int code;

	if (!msp_exp_state.initialized) {
		/* Check that we are initialized */
		msp_exp_state_initialize(msp_seqflags_init());
	} else if (msp_exp_state.busy) {
		/* If we are busy, send a header telling the OBC that we are in the
		 * process of handling a previous packet. */
		msp_exp_frame_format_empty_header(data, MSP_OP_EXP_BUSY);
		*len = 9; /* length of header frame = 9 */

		return MSP_EXP_ERR_IS_BUSY;
	}

	/* Now mark the MSP state as busy and carry on */
	msp_exp_state.busy = 1;
	code = handle_outgoing_frame(data, len);
	msp_exp_state.busy = 0;

	return code;
}



/*---------------------------------------------------------------------------*/
/*                       FUNCTIONS FOR INCOMING FRAMES                       */
/*---------------------------------------------------------------------------*/

/*
 * Handles an incoming frame. Recall that if the experiment receives a faulty
 * frame, it should discard it with no further action. In this case, an error
 * code is returned without further action.
 *
 * Arguments
 *  frame: The received bytes that make up the frame.
 *  len: The number of bytes received.
 */
static int handle_incoming_frame(const unsigned char *frame, unsigned long len)
{
	unsigned char opcode;
	unsigned char frame_id;
	unsigned long dl;

	opcode = frame[0] & 0x7F;
	frame_id = (frame[0] >> 7) & 0x1;

	if (opcode == MSP_OP_DATA_FRAME) {
		/* Check that the data frame has a correct length */
		if (len < 6 || len > MSP_EXP_MAX_FRAME_SIZE)
			return MSP_EXP_ERR_INVALID_DATA_FRAME;
		else
			return handle_incoming_data_frame(frame + 1, frame_id, len - 5);
	} else {
		/* Check that the header frame has length 9 */
		if (len != 9) {
			return MSP_EXP_ERR_INVALID_HEADER_FRAME;
		} else {
			dl = msp_from_bigendian32(frame + 1);
			return handle_incoming_header_frame(opcode, frame_id, dl);
		}
	}
}
/*
 * Handles an incoming data frame.
 *
 * NOTE ABOUT THE IMPLEMENTATION OF THIS FUNCTION:
 * It is tempting to make sure that we have acknowledged the previous data
 * frame before accepting another one. But by the MSP protocol, the OBC must
 * wait for an acknowledgment before sending the next data frame. While we
 * could add a state variable to check if the previous frame has been acked or
 * not, it would most likely not help in this case since something must have
 * gone badly wrong with the OBC. For example, if it sends 3 data frames in a
 * row, we will have no idea that the third frame is different from the first
 * frame since their frame-ID's are identical (unless we start comparing frames
 * by storing the FCS or the last acknowledged frame). In short; lets trust the
 * OBC implementation of MSP.
 *
 * Arguments
 *  data: Pointer to the data field in the frame. (NOT to the whole frame!)
 *  frame_id: Frame-ID of the frame.
 *  len: Length of the data field in the frame.
 */
static int handle_incoming_data_frame(const unsigned char *data, unsigned char frame_id, unsigned long len)
{
	/* We should only receive data frames in the OBC Send state */
	if (msp_exp_state.type != MSP_EXP_STATE_OBC_SEND_RX)
		return MSP_EXP_ERR_UNEXPECTED_DATA_FRAME;

	/* Check that the frame-ID is different from the previous frame */
	if (frame_id == msp_exp_state.last_received_frame_id)
		return MSP_EXP_ERR_DUPLICATE_FRAME;

	/* Check that we are not receiving more data than we are expecting */
	if (msp_exp_state.processed_length + len > msp_exp_state.total_length)
		return MSP_EXP_ERR_INVALID_DATA_FRAME;

	/* All seems good. This is a data frame that we have previously not
	 * encountered. So we can safely send it up to the exprecv handler. */
	msp_exprecv_data(msp_exp_state.opcode, data, len, msp_exp_state.processed_length);

	/* Update the number of processed bytes */
	msp_exp_state.processed_length += len;
	msp_exp_state.last_received_frame_id = frame_id;

	return 0;
}

/*
 * Handles an incoming header frame.
 *
 * Arguments
 *  opcode: OP-code of the header.
 *  frame_id: Frame-ID of the frame.
 *  dl: The value of the DL field in the frame.
 */
static int handle_incoming_header_frame(unsigned char opcode, unsigned char frame_id, unsigned long dl)
{
	int code;

	/* Check the type of incoming header and let another function handler the
	 * header based on the type. */
	switch (MSP_OP_TYPE(opcode)) {
	case MSP_OP_TYPE_CTRL:
		code = handle_incoming_control_frame(opcode, frame_id);
		break;
	case MSP_OP_TYPE_SYS:
		code = handle_incoming_system_frame(opcode, frame_id);
		break;
	case MSP_OP_TYPE_REQ:
		code = handle_incoming_request_frame(opcode);
		break;
	case MSP_OP_TYPE_SEND:
		code = handle_incoming_send_frame(opcode, frame_id, dl);
		break;
	default:
		code = MSP_EXP_ERR_FAULTY_FRAME;
		msp_debug_hex("Unknown opcode type for ", opcode);
		break;
	}

	return code;
}
/*
 * Handles an incoming control flow header frame.
 * 
 * Arguments
 *  opcode: OP-code of the header.
 *  frame_id: Frame-ID of the frame.
 */
static int handle_incoming_control_frame(unsigned char opcode, unsigned char frame_id)
{
	int code;

	switch (opcode) {
	case MSP_OP_NULL:
		/* If we received a NULL frame, we should always go back to the READY
		 * state. If we were in a state previously, we need to send and error
		 * notifying that we have aborted the transaction. */
		ensure_ready_state();
		code = 0;
		break;
	case MSP_OP_F_ACK:
		if (msp_exp_state.processed_length + msp_exp_state.prev_data_length >= msp_exp_state.total_length) {
			/* We should get T_ACK in this situation */
			code = MSP_EXP_ERR_FAULTY_FRAME;
			msp_debug("Got an F_ACK when we should've gotten a T_ACK");
		} else if (frame_id != msp_exp_state.frame_id) {
			code = MSP_EXP_ERR_FAULTY_FRAME;
			msp_debug("Frame-ID of F_ACK does not match up with last sent frame.");
		} else if (msp_exp_state.type == MSP_EXP_STATE_OBC_REQ_RESPONSE) {
			/* Response Acknowledged, start transmission of data. */
			msp_exp_state.processed_length = 0;
			msp_exp_state.frame_id ^= 1;
			msp_exp_state.type = MSP_EXP_STATE_OBC_REQ_TX;
			code = 0;
		} else if (msp_exp_state.type == MSP_EXP_STATE_OBC_REQ_TX) {
			/* Data frame acknowledged, prepare the next data */
			msp_exp_state.processed_length += msp_exp_state.prev_data_length;
			msp_exp_state.frame_id ^= 1;
			code = 0;
		} else {
			code = MSP_EXP_ERR_FAULTY_FRAME;
			msp_debug("Received F_ACK when not sending any data.");
		}
		break;
	case MSP_OP_T_ACK:
		/* We should only get this frame if we are in an OBC Request situation. */
		if (!(msp_exp_state.type == MSP_EXP_STATE_OBC_REQ_RESPONSE ||
			  msp_exp_state.type == MSP_EXP_STATE_OBC_REQ_TX)) {
			code = MSP_EXP_ERR_FAULTY_FRAME;
			msp_debug("Received T_ACK when not in a request state.");
		} else if (frame_id != msp_exp_state.transaction_id) {
			/* The transaction ID of the transaction does not match up with
			 * the T_ACK. */
			code = MSP_EXP_ERR_FAULTY_FRAME;
			msp_debug_int("T_ACK should have frame-ID ", msp_exp_state.transaction_id);
		} else {
			/* Transaction Acknowledged. Call the handler function, increment
			 * the sequence flag, and move to the Ready state. */
			msp_expsend_complete(msp_exp_state.opcode);
			msp_seqflags_set(&msp_exp_state.seqflags, msp_exp_state.opcode, frame_id);
			msp_exp_state.type = MSP_EXP_STATE_READY;
			code = 0;
		}
		break;
	default:
		code = MSP_EXP_ERR_FAULTY_FRAME;
		msp_debug_hex("Received unhandlable control flow opcode: ", opcode);
		break;
	}

	return code;
}
/*
 * Handles an incoming system control header frame.
 * 
 * Arguments
 *  opcode: OP-code of the frame.
 *  frame_id: Frame-ID of the frame.
 */
static int handle_incoming_system_frame(unsigned char opcode, unsigned char frame_id)
{
	ensure_ready_state();

	msp_exp_state.transaction_id = frame_id;
	msp_exp_state.last_received_frame_id = frame_id;
	msp_exp_state.opcode = opcode;
	msp_exp_state.total_length = 0;
	msp_exp_state.processed_length = 0;
	msp_exp_state.prev_data_length = 0;

	/* Do not call handler here, wait until we have acknowledged the
	 * transaction and that it is different from the previous transaction. */

	/* Set the MSP state */
	if (msp_seqflags_is_set(&msp_exp_state.seqflags, opcode, frame_id)) {
		msp_exp_state.type = MSP_EXP_STATE_OBC_SEND_RX_DUPLICATE;
	} else {
		msp_exp_state.type = MSP_EXP_STATE_OBC_SEND_RX;
	}

	return 0;
}
/*
 * Handles an incoming request header frame.
 * 
 * Arguments
 *  opcode: OP-code of the frame.
 */
static int handle_incoming_request_frame(unsigned char opcode)
{
	unsigned long data_to_send;

	ensure_ready_state();

	msp_exp_state.transaction_id = msp_seqflags_get_next(&msp_exp_state.seqflags, opcode);
	msp_exp_state.frame_id = msp_exp_state.transaction_id;
	msp_exp_state.opcode = opcode;
	msp_exp_state.processed_length = 0;
	msp_exp_state.prev_data_length = 0;

	data_to_send = 0;
	msp_expsend_start(opcode, &data_to_send);
	
	msp_exp_state.total_length = data_to_send;

	/* OBC Send state */
	msp_exp_state.type = MSP_EXP_STATE_OBC_REQ_RESPONSE;

	return 0;
}
/*
 * Handles an incoming send header frame.
 * 
 * Arguments
 *  opcode: OP-code of the header.
 *  frame_id: Frame-ID of the frame.
 *  dl: The value of the DL field in the frame.
 */
static int handle_incoming_send_frame(unsigned char opcode, unsigned char frame_id, unsigned long dl)
{
	ensure_ready_state();

	msp_exp_state.transaction_id = frame_id;
	msp_exp_state.last_received_frame_id = frame_id;
	msp_exp_state.opcode = opcode;
	msp_exp_state.total_length = dl;
	msp_exp_state.processed_length = 0;

	/* Set the MSP state */
	if (msp_seqflags_is_set(&msp_exp_state.seqflags, opcode, frame_id)) {
		msp_exp_state.type = MSP_EXP_STATE_OBC_SEND_RX_DUPLICATE;
	} else {
		msp_exp_state.type = MSP_EXP_STATE_OBC_SEND_RX;
		
		/* If this is not a duplicate, then we call the appropriate handler to
		 * setup all the data. */
		msp_exprecv_start(opcode, dl);
	}

	return 0;
}




/*---------------------------------------------------------------------------*/
/*                       FUNCTIONS FOR OUTGOING FRAMES                       */
/*---------------------------------------------------------------------------*/

/*
 * Handles an outgoing frame.
 *
 * Arguments
 *  buf: Pointer to the buffer where the frame will be stored.
 *  len: Pointer to an integer which represents the length of the outgoing 
 *       data.
 */
static int handle_outgoing_frame(unsigned char *buf, unsigned long *len)
{
	int code;

	switch (msp_exp_state.type) {
	case MSP_EXP_STATE_READY:
		msp_exp_frame_format_empty_header(buf, MSP_OP_NULL);
		*len = 9;
		code = 0;
		break;
	case MSP_EXP_STATE_OBC_REQ_RESPONSE:
		code = handle_outgoing_response_frame(buf, len);
		break;
	case MSP_EXP_STATE_OBC_REQ_TX:
		code = handle_outgoing_data_frame(buf, len);
		break;
	case MSP_EXP_STATE_OBC_SEND_RX:
	case MSP_EXP_STATE_OBC_SEND_RX_DUPLICATE:
		code = handle_outgoing_acknowledge_frame(buf, len);
		break;
	default:
		/* If we are in some form of erroneous state, go into the Ready state
		 * and send a NULL frame. */
		ensure_ready_state();
		msp_exp_frame_format_empty_header(buf, MSP_OP_NULL);
		*len = 9;
		code = MSP_EXP_ERR_STATE_ERROR;
		msp_debug("invalid state type");
		break;
	}

	return code;
}
/*
 * Handles an outgoing response frame to an OBC Request.
 *
 * Arguments
 *  buf: Pointer to the buffer where the frame will be stored.
 *  len: Pointer to an integer which represents the length of the outgoing 
 *       data.
 */
static int handle_outgoing_response_frame(unsigned char *buf, unsigned long *len)
{
	/* Format a header saying how much we are going to send. State and frame-ID
	 * is only updated first when we receive an acknowledge frame. */
	msp_exp_frame_format_header(buf, MSP_OP_EXP_SEND, msp_exp_state.transaction_id, msp_exp_state.total_length);
	*len = 9;

	return 0;
}
/*
 * Handles an outgoing data frame.
 *
 * Arguments
 *  buf: Pointer to the buffer where the frame will be stored.
 *  len: Pointer to an integer which represents the length of the outgoing 
 *       data.
 */
static int handle_outgoing_data_frame(unsigned char *buf, unsigned long *len)
{
	unsigned long send_len, remaining_len;
	unsigned long fcs;

	/* If we have nothing left to send, something has gone very wrong. Send a
	 * NULL frame to the OBC and go to the ready state. */
	if (msp_exp_state.processed_length >= msp_exp_state.total_length) {
		ensure_ready_state();
		msp_exp_frame_format_empty_header(buf, MSP_OP_NULL);
		*len = 9;
		msp_debug("trying to send empty data frame");
		return MSP_EXP_ERR_STATE_ERROR;
	}

	/* Calculate how many bytes that are to be sent. */
	send_len = MSP_EXP_MTU;
	remaining_len = msp_exp_state.total_length - msp_exp_state.processed_length;
	if (remaining_len < MSP_EXP_MTU) {
		send_len = remaining_len;
	}

	/* This is needed for when we receive acknowledgments */
	msp_exp_state.prev_data_length = send_len;

	/* All good, now lets fill up the buffer with data. */
	buf[0] = MSP_OP_DATA_FRAME | (msp_exp_state.frame_id << 7);
	msp_expsend_data(msp_exp_state.opcode, buf + 1, send_len, msp_exp_state.processed_length);

	/* Generate and format the Frame Check Sequence (from_obc = 0) */
	fcs = msp_exp_frame_generate_fcs(buf, 0, send_len+1);
	msp_to_bigendian32(buf + (send_len + 1), fcs);

	/* Set the total length of the frame */
	*len = send_len+5;

	return 0;
}
/*
 * Handles an outgoing acknowledge frame. Also handles the case where we
 * receive a duplicate OBC Send transaction.
 *
 * Arguments
 *  buf: Pointer to the buffer where the frame will be stored.
 *  len: Pointer to an integer which represents the length of the outgoing 
 *       data.
 */
static int handle_outgoing_acknowledge_frame(unsigned char *buf, unsigned long *len)
{
	int code;
	unsigned char opcode;
	unsigned char transaction_id;

	if (msp_exp_state.type == MSP_EXP_STATE_OBC_SEND_RX_DUPLICATE) {
		msp_exp_frame_format_header(buf, MSP_OP_T_ACK, msp_exp_state.transaction_id, 0);
		*len = 9;
		msp_exp_state.type = MSP_EXP_STATE_READY;
		return 0;
	}

	code = 0;

	/* Now check if we need to T_ACK an actual transaction */
	if (msp_exp_state.processed_length >= msp_exp_state.total_length) {
		msp_exp_frame_format_header(buf, MSP_OP_T_ACK, msp_exp_state.transaction_id, 0);
		*len = 9;
		msp_exp_state.type = MSP_EXP_STATE_READY;

		opcode = msp_exp_state.opcode;
		transaction_id = msp_exp_state.transaction_id;

		switch (MSP_OP_TYPE(msp_exp_state.opcode)) {
		case MSP_OP_TYPE_SYS:
			msp_exprecv_syscommand(msp_exp_state.opcode);
			msp_seqflags_set(&msp_exp_state.seqflags, opcode, transaction_id);
			break;
		case MSP_OP_TYPE_SEND:
			msp_exprecv_complete(msp_exp_state.opcode);
			msp_seqflags_set(&msp_exp_state.seqflags, opcode, transaction_id);
			break;
		default:
			msp_debug("Trying to send T_ACK in non OBC Send state");
			code = MSP_EXP_ERR_STATE_ERROR;
			break;
		}
	} else {
		/* Acknowledge a single frame */
		msp_exp_frame_format_header(buf, MSP_OP_F_ACK, msp_exp_state.last_received_frame_id, 0);
		*len = 9;
	}

	return code;
}




/*
 * Ensures that MSP is in the ready state. This means that if a current
 * transaction is active, it will be aborted.
 */
static void ensure_ready_state(void)
{
	switch (msp_exp_state.type) {
	case MSP_EXP_STATE_OBC_SEND_RX:
		/* System Control OP codes are an exception as they have a special
		 * handler. */
		if (MSP_OP_TYPE(msp_exp_state.opcode) != MSP_OP_TYPE_SYS)
			msp_exprecv_error(msp_exp_state.opcode, MSP_EXP_ERR_TRANSACTION_ABORTED);
		break;
	case MSP_EXP_STATE_OBC_REQ_RESPONSE:
	case MSP_EXP_STATE_OBC_REQ_TX:
		msp_expsend_error(msp_exp_state.opcode, MSP_EXP_ERR_TRANSACTION_ABORTED);
		break;
	default:
		/* If we were in a state of a duplicate transaction or simply in the
		 * ready state, we don't need to report any errors. */
		break;
	}

	msp_exp_state.type = MSP_EXP_STATE_READY;
}
