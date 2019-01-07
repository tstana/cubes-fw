/*
 * msp_sequence.h
 * Author: John Wikman
 *
 * Description:
 * Each type of request/send has exactly one sequence flag
 * associated with it. Each flag is represented by one bit.
 *
 * The sequence flag represents what value the sequence flag
 * in the header frame that specifies how much data is going
 * to be sent. The frames have the following OP CODEs:
 * - MSP_EXP_SEND_DATA
 * - MSP_OBD_SEND_PUS
 * - MSP_OBC_SEND_TIME
 * - MSP_OBC_ACTIVE
 * - MSP_OBC_SLEEP
 * - MSP_OBC_POWER_OFF
 *
 * Each sequence flag is bound to that OP CODE with the
 * exception of MSP_EXP_SEND_DATA. The sequence flag associated
 * with the frame that has OP CODE MSP_EXP_SEND_DATA is
 * determined by the previous frame with an OP CODE corresponding
 * to a request. These OP CODEs are:
 * - MSP_OBC_REQ_HK
 * - MSP_OBC_REQ_PUS
 * - MSP_OBC_REQ_PAYLOAD
 *
 * Example:
 * - OBC sends header frame with OP CODE: MSP_OBC_REQ_PUS
 * - Experiment sends header frame with OP CODE: MSP_EXP_SEND_DATA
 *   - The sequence bit in this frame should correspond to the
 *     value of: MSP_seqflags_get(MSP_OBC_REQ_PUS).
 */

#ifndef MSP_SEQUENCE_H
#define MSP_SEQUENCE_H

typedef struct {
	unsigned short values[4];
	unsigned short inits[4];
} msp_seqflags_t;

/**
 * Creates a new set of sequence flags. Use MSP_seqflagsIncrement to set
 * specific flags to 1.
 *
 * @return 	A set of sequence flags with all flags set to 0.
 */
msp_seqflags_t msp_seqflags_init(void);


int msp_seqflags_increment(volatile msp_seqflags_t *flags, unsigned char opcode);

/**
 * Returns the current value of the sequence flag associated with
 * the entered OP CODE.
 * 
 * @return	0	if the associated flag is set to 0.
 *			1	if the associated flag is set to 1.
 *			-1	if the entered op code does not have an
 *				associated sequence flag.
 */
int msp_seqflags_get(volatile const msp_seqflags_t *flags, unsigned char opcode);

/*
 * Returns the next unacknowledged sequence flag for the given OP code.
 */
int msp_seqflags_get_next(volatile const msp_seqflags_t *flags, unsigned char opcode);

/**
 * Returns 1 if the seqflags is set for the entered op code. Else
 * returns 0.
 */
int msp_seqflags_is_set(volatile const msp_seqflags_t *flags, unsigned char opcode, unsigned char flag);

/**
 * Returns 0 if the seqflag was set successfully. Returns -1 if
 * OP CODE has no sequence flag.
 */
int msp_seqflags_set(volatile msp_seqflags_t *flags, unsigned char opcode, unsigned char flag);

#endif