/*
 * msp_exp_frame.c
 * Author: John Wikman
 */

#include "msp_crc.h"
#include "msp_endian.h"
#include "msp_exp_frame.h"

#include "msp_exp_definitions.h"

/*
 * Constructs an FCS based on the entered data. Also includes the pseudo header
 * in the calculation.
 *
 * Arguments
 *  data: Pointer to the all of the bytes to be included in the FCS calculation
 *        except for the pseudo header.
 *  from_obc: A boolean value that specifies whether this frame is supposed to
 *            come from the OBC or not.
 *  len: Number of bytes in data that should be included in the FCS
 *       calculation.
 */
unsigned long msp_exp_frame_generate_fcs(const unsigned char *data, int from_obc, unsigned long len)
{
	unsigned char pseudo_header;
	unsigned long remainder;

	/* Format the pseudo header */
	pseudo_header = (MSP_EXP_ADDR) << 1;
	if (!from_obc)
		pseudo_header |= 0x01;

	remainder = crc32(&pseudo_header, 1, 0);

	/* Now account for the rest of the frame */
	remainder = crc32(data, len, remainder);

	return remainder;
}

/*
 * Check if the MSP frame stored in the pointers data has a valid FCS.
 *
 * Arguments
 *  data: Pointer to the data that contains the MSP frame.
 *  from_obc: A boolean value that specifies whether this frame is supposed to
 *            come from the OBC or not.
 *  len: Length of the data that contains the MSP frame (in bytes).
 *
 * Returns
 *  1 if valid, 0 otherwise.
 */
int msp_exp_frame_fcs_valid(const unsigned char *data, int from_obc, unsigned long len)
{
	unsigned long fcs;

	/* FCS is the last 4 bytes of the frame */
	fcs = from_bigendian32(data + (len - 4));

	/* Check if the FCS' match up. */
	if (fcs == msp_exp_frame_generate_fcs(data, from_obc, len - 4))
		return 1;
	else
		return 0;
}


/*
 * Formats a header with the specified opcode, frame-ID and DL. The result is
 * stored in dest.
 *
 * Arguments
 *  dest: Address to the buffer where the frame will be stored.
 *  opcode: The opcode of the resulting frame.
 *  frame_id: The frame-ID of the resulting frame.
 *  dl: The value of the DL field in the resulting frame.
 */
void msp_exp_frame_format_header(unsigned char *dest, unsigned char opcode, unsigned char frame_id, unsigned long dl)
{
	unsigned long fcs;

	/* Format OP code and Frame-ID */
	dest[0] = opcode & 0x7F;
	dest[0] |= (frame_id & 0x1) << 7;

	/* Format the DL field */
	to_bigendian32(dest + 1, dl);

	/* Format the FCS field */
	fcs = msp_exp_frame_generate_fcs(dest, 0, 5);
	to_bigendian32(dest + 5, fcs);
}

/*
 * Formats a header that has the specified opcode, but with frame-ID and DL set
 * to 0. The resulting frame is stored in dest.
 *
 * Same as calling:
 * msp_exp_frame_format_header(dest, opcode, 0, 0);
 *
 * Arguments
 *  dest: Address to the buffer where the frame will be stored.
 *  opcode: The opcode of the resulting frame.
 */
void msp_exp_frame_format_empty_header(unsigned char *dest, unsigned char opcode)
{
	msp_exp_frame_format_header(dest, opcode, 0, 0);
}
