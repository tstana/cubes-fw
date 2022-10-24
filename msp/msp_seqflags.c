/**
 * @file      msp_seqflags.c
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Defines functions for keeping track of transaction-ID's in MSP.
 *
 * @details
 * A sequence flag keeps track of the transaction-ID from the last successful
 * MSP transaction. Every request, send and system command opcodes has a
 * designated sequence flag. This also goes for custom opcodes.
 */

#include "msp_opcodes.h"
#include "msp_seqflags.h"

struct msp_flag_position {
	int index;
	unsigned short mask;
};

static struct msp_flag_position msp_get_flag_pos(unsigned char opcode);

/**
 * @brief Creates a new (blank) set of sequence flags.
 * @return A new set of sequence flags.
 *
 * Creates a new set of sequence flags with all flags being uninitialized and
 * having their value set to 0.
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
 * @brief Increments a sequence flag.
 * @param flags Pointer to the set of sequence flags.
 * @param opcode The opcode corresponding to the sequence flag.
 * @return 1 if the opcode has no designated sequence flag, 0 otherwise.
 *
 * Increments a sequence flag an initializes it. If the specified opcode has no
 * designated sequence flag, the value 1 is returned and no action is taken.
 */
int msp_seqflags_increment(volatile msp_seqflags_t *flags, unsigned char opcode)
{
	struct msp_flag_position fp;

	fp = msp_get_flag_pos(opcode);
	if (fp.mask == 0)
		return 1;

	flags->values[fp.index] ^= fp.mask;
	flags->inits[fp.index] |= fp.mask;

	return 0;
}

/**
 * @brief Returns the value of a sequence flag.
 * @param flags Pointer to the set of sequence flags.
 * @param opcode The opcode corresponding to the sequence flag.
 * @return
 *    -  -1 if the opcode has no corresponding sequence flag,
 *    -  0 if the sequence flag is set to 0,
 *    -  1 if the sequence flag is set to 1.
 *
 * Returns the value of the sequence flag associated with the opcode. Returns
 * -1 if the specified opcode is not associated with a sequence flag.
 *
 * Since this function can return -1, it is important to check the returned
 * value against -1 or make sure that the opcode is always valid.
 */
int msp_seqflags_get(volatile const msp_seqflags_t *flags, unsigned char opcode)
{
	struct msp_flag_position fp;

	fp = msp_get_flag_pos(opcode);
	if (fp.mask == 0)
		return -1;

	/* Check the flag */
	if(flags->values[fp.index] & fp.mask)
		return 1;
	else
		return 0;
}

/**
 * @brief Returns the next expected value of a sequence flag.
 * @param flags Pointer to the set of sequence flags.
 * @param opcode The opcode corresponding to the sequence flag.
 * @return
 *    -  -1 if the opcode has no corresponding sequence flag,
 *    -  0 if the sequence flag is uninitialized or if it is set to 1,
 *    -  1 if the sequence flag is initialized and currently set to 0.
 *
 * Returns the next expected sequence flag. In the case of MSP, it returns the
 * transaction-ID of the next transaction with the specified opcode.
 *
 * Since this function can return -1, it is important to check the returned
 * value against -1 or make sure that the opcode is always valid.
 */
int msp_seqflags_get_next(volatile const msp_seqflags_t *flags, unsigned char opcode)
{
	struct msp_flag_position fp;

	fp = msp_get_flag_pos(opcode);
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
 * @brief Checks if a sequence flag is set to a specific value.
 * @param flags Pointer to the set of sequence flags.
 * @param opcode The opcode corresponding to the sequence flag.
 * @param flag The value to check the sequence flag against.
 * @return 1 if the value of the sequence flag matches the value in the flag
 *         parameter, 0 otherwise.
 *
 * Checks if the sequence flag corresponding to the specified opcode is equal
 * to a specific value. Returns 1 if the values are equal. Returns 0 if the
 * specified opcode does not have a corresponding sequence flag or if the
 * values are not equal.
 */
int msp_seqflags_is_set(volatile const msp_seqflags_t *flags, unsigned char opcode, unsigned char flag)
{
	struct msp_flag_position fp;

	fp = msp_get_flag_pos(opcode);
	if (fp.mask == 0)
		return 0;

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
 * @brief Sets a sequence flag to a specific value.
 * @param flags Pointer to the set of sequence flags.
 * @param opcode The opcode corresponding to the sequence flag.
 * @param flag The value that the sequence flag should be set to.
 * @return -1 if the opcode does not have a designated sequence flag, 0
 *         otherwise.
 *
 * Sets the sequence flag corresponding to the opcode to the value specified
 * by the flag parameter. If the flag parameter is 0, the sequence flag will be
 * set to 0. If the flag parameter is any other value, the sequence flag will
 * be set to 1.
 */
int msp_seqflags_set(volatile msp_seqflags_t *flags, unsigned char opcode, unsigned char flag)
{
	struct msp_flag_position fp;

	fp = msp_get_flag_pos(opcode);
	if (fp.mask == 0)
		return -1;

	/* Initialize the flag */
	flags->inits[fp.index] |= fp.mask;

	if (flag)
		flags->values[fp.index] |= fp.mask;
	else
		flags->values[fp.index] &= ~fp.mask;

	return 0;
}


/*
 * Returns flag position information for the specified opcode. The mask field
 * is set to 0 if the opcode does not have an associated flag.
 */
static struct msp_flag_position msp_get_flag_pos(unsigned char opcode)
{
	struct msp_flag_position fp;

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
