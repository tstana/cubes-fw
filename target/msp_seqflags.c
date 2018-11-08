/*
 * msp_seqflags.c
 * Author: John Wikman
 */

#include "msp_opcodes.h"
#include "msp_seqflags.h"

struct flag_position {
	int index;
	unsigned short mask;
};

/**
 * Returns flag position information for the specified opcode. The mask field
 * is set to 0 if the opcode does not have an associated flag.
 */
static struct flag_position get_flag_pos(unsigned char opcode);

/**
 * Creates a new set of sequence flags. Use MSP_seqflagsIncrement to set
 * specific flags to 1.
 *
 * @return  A set of sequence flags with all flags set to 0.
 */
msp_seqflags_t msp_seqflags_init(void)
{
	msp_seqflags_t flags;
	flags.values[0] = 0;
	flags.values[1] = 0;
	flags.values[2] = 0;
	flags.values[3] = 0;
	flags.inits[0] = 0;
	flags.inits[1] = 0;
	flags.inits[2] = 0;
	flags.inits[3] = 0;
	
	return flags;
}

/**
 * Increments the sequence flag belonging to that specific OP
 * CODE.
 *
 * @return  0 if corresponding flag was incremented.
 *      1 if the entered op code does not have an associated
 *        sequence flag.
 */
int msp_seqflags_increment(volatile msp_seqflags_t *flags, unsigned char opcode)
{
	struct flag_position fp;

	fp = get_flag_pos(opcode);
	if (fp.mask == 0)
		return 1;

	flags->values[fp.index] ^= fp.mask;
	flags->inits[fp.index] |= fp.mask;

	return 0;
}

/**
 * Returns the current value of the sequence flag associated with
 * the entered OP CODE.
 * 
 * @return  0 if the associated flag is set to 0.
 *      1 if the associated flag is set to 1.
 *      -1  if the entered op code does not have an
 *        associated sequence flag.
 */
int msp_seqflags_get(volatile const msp_seqflags_t *flags, unsigned char opcode)
{
	struct flag_position fp;

	fp = get_flag_pos(opcode);
	if (fp.mask == 0)
		return -1;

	/* Check the flag */
	if(flags->values[fp.index] & fp.mask)
		return 1;
	else
		return 0;
}

/*
 * Returns the next unacknowledged sequence flag for the given OP code. If the
 * flag for the given opcode is not initialized, it returns 0.
 */
int msp_seqflags_get_next(volatile const msp_seqflags_t *flags, unsigned char opcode)
{
	struct flag_position fp;

	fp = get_flag_pos(opcode);
	if (fp.mask == 0)
		return -1;

	if ((flags->inits[fp.index] & fp.mask) == 0)
		return 0;

	/* Returns the opposite (next value). If flag is 1 it returns 0 and vice
	 * versa */
	if(flags->values[fp.index] & fp.mask)
		return 0;
	else
		return 1;
}

/**
 * Returns 1 if the seqflags is set to the specific flag for the entered op
 * code. Else returns 0. Returns -1 if OP CODE has no sequence flag.
 */
int msp_seqflags_is_set(volatile const msp_seqflags_t *flags, unsigned char opcode, unsigned char flag)
{
	struct flag_position fp;

	fp = get_flag_pos(opcode);
	if (fp.mask == 0)
		return -1;

	/* Make sure that the flag has been set at least once */
	if ((flags->inits[fp.index] & fp.mask) == 0)
		return 0;

	/* Make sure that the flag is 0 or 1 */
	if (flag)
		flag = 1;

	/* Check if the flags are equal */
	if (flag == msp_seqflags_get(flags, opcode))
		return 1;
	else
		return 0;
}

/**
 * Returns 0 if the seqflag was set successfully. Returns -1 if
 * OP CODE has no sequence flag.
 */
int msp_seqflags_set(volatile msp_seqflags_t *flags, unsigned char opcode, unsigned char flag)
{
	struct flag_position fp;

	fp = get_flag_pos(opcode);
	if (fp.mask == 0)
		return 1;

	/* Initialize the flag */
	flags->inits[fp.index] |= fp.mask;

	if (flag)
		flags->values[fp.index] |= fp.mask;
	else
		flags->values[fp.index] &= ~fp.mask;

	return 0;
}


/* Retrieves flag position information for the specified opcode */
static struct flag_position get_flag_pos(unsigned char opcode)
{
	struct flag_position fp;

	if (MSP_OP_IS_CUSTOM(opcode)) {
		switch (MSP_OP_TYPE(opcode)) {
		case MSP_OP_TYPE_SYS:
			fp.index = 1;
			break;
		case MSP_OP_TYPE_REQ:
			fp.index = 2;
			break;
		case MSP_OP_TYPE_SEND:
			fp.index = 3;
			break;
		}

		fp.mask = 1 << (opcode & 0x0F);
	} else {
		fp.index = 0;

		switch (opcode) {
		case MSP_OP_ACTIVE:
			fp.mask = 0x0001;
			break;
		case MSP_OP_SLEEP:
			fp.mask = 0x0002;
			break;
		case MSP_OP_POWER_OFF:
			fp.mask = 0x0004;
			break;
		case MSP_OP_REQ_PAYLOAD:
			fp.mask = 0x0008;
			break;
		case MSP_OP_REQ_HK:
			fp.mask = 0x0010;
			break;
		case MSP_OP_REQ_PUS:
			fp.mask = 0x0020;
			break;
		case MSP_OP_SEND_TIME:
			fp.mask = 0x0040;
			break;
		case MSP_OP_SEND_PUS:
			fp.mask = 0x0080;
			break;
		default:
			fp.mask = 0;
			break;
		}
	}

	return fp;
}
