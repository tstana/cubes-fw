/*
 * msp_exp_callback.h
 * Author: John Wikman
 *
 * Contains the callback functions for MSP. Whenever an the OBC calls upon the
 * experiment, one of these functions must be called.
 */

#ifndef MSP_EXP_CALLBACK_H
#define MSP_EXP_CALLBACK_H

/*
 * Callback function for when receiving data from the OBC.
 *
 * Arguments
 *  data: Pointer to the received data from the OBC. 
 *  len: Number of bytes received from the OBC.
 */
int msp_recv_callback(const unsigned char *data, unsigned long len);

/*
 * Callback function for when the OBC is requesting data from the experiment.
 *
 * Arguments
 *  data: Pointer to a buffer where the data to be sent will be stored. The
 *        buffer must be at least MSP_MTU+5 in size. (Make sure to set MSP_MTU
 *        in msp_exp_definitions.h)
 *  len: A pointer to a 32-bit unsigned int that represents the number of bytes
 *       to be sent.
 */
int msp_send_callback(unsigned char *data, unsigned long *len);

#endif
