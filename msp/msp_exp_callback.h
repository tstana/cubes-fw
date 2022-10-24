/**
 * @file      msp_exp_callback.h
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Declares the callback functions for the experiment side of MSP.
 *
 * @details
 * Contains the callback functions for MSP. Whenever the OBC calls upon the
 * experiment, one of these functions must be called.
 */

#ifndef MSP_EXP_CALLBACK_H
#define MSP_EXP_CALLBACK_H

/**
 * @brief Callback function for when receiving data from the OBC.
 * @param data Pointer to the received data from the OBC.
 * @param len Number of bytes received from the OBC.
 * @return 0 if OK, otherwise an error code from msp_exp_error.h.
 */
int msp_recv_callback(const unsigned char *data, unsigned long len);

/**
 * @brief Callback function for when the OBC is requesting data from the
 *        experiment.
 * @param data Pointer to a buffer where the data to be sent will be stored.
 *             The buffer must be at least MSP_EXP_MAX_FRAME_SIZE in size. If
 *             the conf.py setup script is used, this constant should be set in
 *             msp_exp_definitions.h.
 * @param len A pointer to a 32-bit unsigned integer that represents the number
 *            of bytes to be sent.
 * @return 0 if OK, otherwise an error code from msp_exp_error.h.
 */
int msp_send_callback(unsigned char *data, unsigned long *len);

#endif
