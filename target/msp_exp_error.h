/*
 * msp_exp_error.h
 * Author: John Wikman
 *
 * Contains definitions for all the internal error codes used for this MSP
 * implementation.
 */

#ifndef MSP_EXP_ERROR_H
#define MSP_EXP_ERROR_H

/* Callback Errors */
#define MSP_EXP_ERR_IS_BUSY -1
#define MSP_EXP_ERR_FCS_MISMATCH -2
#define MSP_EXP_ERR_INVALID_HEADER_FRAME -3
#define MSP_EXP_ERR_INVALID_DATA_FRAME -4
#define MSP_EXP_ERR_UNEXPECTED_DATA_FRAME -5
#define MSP_EXP_ERR_DUPLICATE_FRAME -6
#define MSP_EXP_ERR_FAULTY_FRAME -7
#define MSP_EXP_ERR_STATE_ERROR -8

/* Transaction Errors */
#define MSP_EXP_ERR_RECEIVED_NULL_FRAME -1
#define MSP_EXP_ERR_TRANSACTION_ABORTED -2
#define MSP_EXP_ERR_TOO_MUCH_DATA_TO_SEND -3

#endif
