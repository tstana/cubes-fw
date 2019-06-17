/*
 * msp_exp_handler.h
 * Author: John Wikman
 * 
 * Provides prototypes for all functions that an experiment will use to
 * interface against MSP.
 *
 * For intuitive reasons, expsend is used instead of obcreq and exprecv is
 * used instead of obcsend.
 *
 * These functions will always be called in a certain order. For example like
 * this:
 *   1. msp_exprecv_start()
 *   2. msp_exprecv_data()
 *   3. msp_exprecv_data()
 *   4. msp_exprecv_complete()
 *   5. msp_exprecv_start()
 *   6. msp_exprecv_data()
 *   7. msp_exprecv_complete()
 *   8. msp_expsend_start()
 *   9. msp_expsend_data()
 *  10. msp_expsend_complete()
 *
 * Or like this (in case of errors):
 *   1. msp_exprecv_start()
 *   2. msp_exprecv_data()
 *   3. msp_exprecv_error()
 *   4. msp_exprecv_start()
 *   5. msp_exprecv_data()
 *   6. msp_exprecv_data()
 *   7. msp_exprecv_complete()
 *   8. msp_exprecv_syscommand()
 *   9. msp_exprecv_syscommand()
 *  10. msp_exprecv_start()
 *  11. msp_exprecv_data()
 *  12. msp_exprecv_complete()
 *
 * In short, each function with the "_start" suffix is called at the start of
 * every transaction to allow for preparation of data and buffers. Each
 * function with the "_complete" suffix is called at the end of every
 * successful transaction. If an error occured during a transaction, the
 * corresponding function with the "_error" suffix is called instead of the
 * function with the "_complete" suffix. The main exception is when a system
 * command is received from the OBC, then msp_exprecv_syscommand is the only
 * function that is called.
 */
#ifndef MSP_EXP_HANDLER_H
#define MSP_EXP_HANDLER_H

/*
 * The function which MSP will call at the start of a transaction when the
 * experiment is to send data to the OBC.
 *
 * Arguments
 *  opcode: The opcode of this transaction, as determined by the OBC.
 *  len: A pointer to a 32-bit unsigned int where the number of bytes to send
 *       to the OBC must be stored.
 */
void msp_expsend_start(unsigned char opcode, unsigned long *len);
/*
 * The function which MSP will call then the experiment is to send a data frame
 * to the OBC.
 *
 * msp_expsend_start will always be called before this function.
 *
 * Arguments
 *  opcode: OP code of the transaction.
 *  buf: Pointer to the buffer where the data to be sent must be stored.
 *  len: The number of bytes that should be put into the buffer.
 *  offset: The number of bytes that have been successfully sent so far.
 */
void msp_expsend_data(unsigned char opcode, unsigned char *buf, unsigned long len, unsigned long offset);
/*
 * The function which MSP will call at the end of a successful transaction
 * where the experiment was sending data to the OBC.
 *
 * Arguments
 *  opcode: The opcode of the finished transaction.
 */
void msp_expsend_complete(unsigned char opcode);
/*
 * This function is called if a transaction where the experiment is sending
 * data to the OBC encountered an error and must be aborted.
 *
 * Arguments
 *  opcode: The opcode of the aborted transaction.
 *  error: The error code. The relevant error codes can be found under
 *         transaction errors in "msp_exp_error.h"
 */
void msp_expsend_error(unsigned char opcode, int error);



/*
 * The function which MSP will call at the start of a transaction when the
 * experiment will receive data from the OBC.
 *
 * Arguments
 *  opcode: The opcode of this transaction, as determined by the OBC.
 *  len: The number of bytes to be received during the entire transaction.
 */
void msp_exprecv_start(unsigned char opcode, unsigned long len);
/*
 * The function which MSP will call when the experiment has received a data
 * frame from the OBC.
 *
 * msp_exprecv_start will always be called before this function.
 *
 * Arguments
 *   opcode: OP code of the transaction.
 *   buf: Pointer to the data received in the data frame. It does NOT include
 *        the data of the frame itself.
 *   len: Length of the data received in the data frame. (Not the length of the
 *        frame itself!)
 *   offset: Number of bytes of data that have been received before this data
 *           frame. Example: If we have previously received 2 data frames with
 *           30 bytes of data each, offset would be set to 60.
 */
void msp_exprecv_data(unsigned char opcode, const unsigned char *buf, unsigned long len, unsigned long offset);
/*
 * The function which MSP will call at the end of a transaction where the
 * experiment was receiving data from the OBC.
 *
 * Arguments
 *  opcode: The opcode of the finished transaction.
 */
void msp_exprecv_complete(unsigned char opcode);
/*
 * This function is called if a transaction where the experiment is receiving
 * data from the OBC encountered an error and must be aborted.
 *
 * Arguments
 *  opcode: The opcode of the aborted transaction.
 *  error: The error code. The relevant error codes can be found under
 *         transaction errors in "msp_exp_error.h"
 */
void msp_exprecv_error(unsigned char opcode, int error);



/*
 * This function is called whenever a system command is received from the OBC.
 * In this case, no other "msp_exprecv" function is called. See the MSP
 * specification to see a list of all the system commands.
 *
 * Arguments
 *  opcode: The opcode corresponding to the system command.
 */
void msp_exprecv_syscommand(unsigned char opcode);


#endif
