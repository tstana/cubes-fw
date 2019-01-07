/*
 * msp_exp_frame.h
 * Author: John Wikman
 *
 * Provides basic functionalities for handling frames on the experiment side of
 * MSP.
 */

#ifndef MSP_FRAME_H
#define MSP_FRAME_H

/* Constructs an FCS based on the input data */
unsigned long msp_exp_frame_generate_fcs(const unsigned char *data, int from_obc, unsigned long len);

/* Check if the FCS of the frame is valid. */
int msp_exp_frame_fcs_valid(const unsigned char *data, int from_obc, unsigned long len);

/* Formats a header with the specified opcode, frame-ID and DL. The result is
 * stored in dest. */
void msp_exp_frame_format_header(unsigned char *dest, unsigned char opcode, unsigned char frame_id, unsigned long dl);

/* Formats a header that has the specified opcode and with frame-ID and dl
 * being set to 0. The result is stored in dest. */
void msp_exp_frame_format_empty_header(unsigned char *dest, unsigned char opcode);

#endif
