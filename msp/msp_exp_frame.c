/**
 * @file      msp_exp_frame.c
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Functions for handling MSP frames on the experiment side.
 */

#include "msp_crc.h"
#include "msp_endian.h"

#include "msp_exp_frame.h"
#include "msp_exp_definitions.h"

/**
 * @brief Calculates the FCS of an MSP frame.
 * @param data Pointer to the all of the bytes to be included in the FCS
 *             calculation, excluding the pseudo header. Should point to the
 *             byte in the MSP frame that contains the opcode and frame-ID.
 * @param from_obc A boolean value that specifies whether the FCS should be
 *                 calculated as if the frame came from the OBC or from the
 *                 experiment. 
 * @param len The number of bytes pointed to be the data parameter. In the
 *            case of a header frame, this should have the value 5. In the case
 *            of a data frame, this should have the value 1 + the length of the
 *            data field.
 * @return The calculated FCS value.
 *
 * Calculates an FCS value based on the entered data.
 */
unsigned long msp_exp_frame_generate_fcs(const unsigned char *data, int from_obc, unsigned long len)
{
	unsigned char pseudo_header;
	unsigned long remainder;

	/* Format the pseudo header */
	pseudo_header = (MSP_EXP_ADDR) << 1;
	if (!from_obc)
		pseudo_header |= 0x01;

	remainder = msp_crc32(&pseudo_header, 1, 0);

	/* Now account for the rest of the frame */
	remainder = msp_crc32(data, len, remainder);

	return remainder;
}

/**
 * @brief Checks if the FCS of an MSP frame is valid.
 * @param data Pointer to the first byte in the MSP frame (the byte that
 *             contains the opcode and frame-ID).
 * @param from_obc A boolean value that specifies whether the FCS should be
 *                 calculated as if the frame came from the OBC or from the
 *                 experiment. 
 * @param len Number of bytes in the MSP frame. In the case of a header frame,
 *            this should have the value 9. In the case of a data frame, this
 *            should have the value 5 + the length of the data field.
 * @return 1 if the FCS is valid, 0 otherwise.
 */
int msp_exp_frame_fcs_valid(const unsigned char *data, int from_obc, unsigned long len)
{
	unsigned long fcs;

	/* FCS is the last 4 bytes of the frame */
	fcs = msp_from_bigendian32(data + (len - 4));

	/* Check if the FCS' match up. */
	if (fcs == msp_exp_frame_generate_fcs(data, from_obc, len - 4))
		return 1;
	else
		return 0;
}


/**
 * @brief Formats a header frame into a sequence of bytes.
 * @param dest Pointer to the buffer where the formatted frame will be stored.
 * @param opcode Opcode of the frame.
 * @param frame_id Frame-ID of the frame.
 * @param dl The value of the DL field.
 *
 * Formats a header with the specified opcode, frame-ID and DL value. The
 * result is stored in the buffer pointed to by dest.
 *
 * This function formats the entire frame, including the FCS value. The
 * resulting sequence of bytes can be sent directly to the OBC.
 */
void msp_exp_frame_format_header(unsigned char *dest, unsigned char opcode, unsigned char frame_id, unsigned long dl)
{
	unsigned long fcs;

	/* Format OP code and Frame-ID */
	dest[0] = opcode & 0x7F;
	dest[0] |= (frame_id & 0x1) << 7;

	/* Format the DL field */
	msp_to_bigendian32(dest + 1, dl);

	/* Format the FCS field */
	fcs = msp_exp_frame_generate_fcs(dest, 0, 5);
	msp_to_bigendian32(dest + 5, fcs);
}

/**
 * @brief Formats a header frame with frame-ID and DL set to 0 into a sequence
 *        of bytes.
 * @param dest Pointer to the buffer where the frame will be stored.
 * @param opcode Opcode of the frame.
 *
 * Same as calling msp_exp_frame_format_header(dest, opcode, 0, 0).
 * 
 * This function should be used for header frames with opcodes that have traits
 * FID0 and DL0.
 */
void msp_exp_frame_format_empty_header(unsigned char *dest, unsigned char opcode)
{
	msp_exp_frame_format_header(dest, opcode, 0, 0);
}
