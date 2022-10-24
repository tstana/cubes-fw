/**
 * @file      msp_exp_handler.h
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Declares all the functions that must be implemented by an
 *            experiment.
 * 
 * @details
 * Provides prototypes for all functions that an experiment must implement to
 * interface against the MSP library. These functions act as callback functions
 * during an MSP transaction.
 *
 * For intuitive reasons, expsend is used instead of obcreq and exprecv is
 * used instead of obcsend.
 *
 * These functions will always be called in a certain order. For example like
 * this:
 *   1.  msp_exprecv_start()
 *   2.  msp_exprecv_data()
 *   3.  msp_exprecv_data()
 *   4.  msp_exprecv_complete()
 *   5.  msp_exprecv_start()
 *   6.  msp_exprecv_data()
 *   7.  msp_exprecv_complete()
 *   8.  msp_expsend_start()
 *   9.  msp_expsend_data()
 *   10. msp_expsend_complete()
 *
 * Or like this (in case of errors):
 *   1.  msp_exprecv_start()
 *   2.  msp_exprecv_data()
 *   3.  msp_exprecv_error()
 *   4.  msp_exprecv_start()
 *   5.  msp_exprecv_data()
 *   6.  msp_exprecv_data()
 *   7.  msp_exprecv_complete()
 *   8.  msp_exprecv_syscommand()
 *   9.  msp_exprecv_syscommand()
 *   10. msp_exprecv_start()
 *   11. msp_exprecv_data()
 *   12. msp_exprecv_complete()
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

/**
 * @brief Called at the start of an OBC Request transaction.
 * @param opcode The opcode of the transaction. (As determined by the OBC.)
 * @param len A pointer to a 32-bit unsigned integer. The number of bytes that
 *            is to be sent to the OBC must be written to the integer pointed
 *            to by this parameter.
 *
 * The function which the MSP library will call at the start of a transaction
 * when the experiment is to send data to the OBC.
 */
void msp_expsend_start(unsigned char opcode, unsigned long *len);

/**
 * @brief Called when a data frame should be filled up during an OBC Request
 *        transaction.
 * @param opcode The opcode of the transaction.
 * @param buf Pointer to the data field in an MSP data frame that should be
 *            filled up with data.
 * @param len How many bytes of data that should be entered into the data
 *            field.
 * @param offset The offset of the data that should be entered into data field.
 *               This number represents the number of bytes that has been
 *               successfully sent in previous data frames in the same
 *               transaction.
 *
 * The function which the MSP library will call then the experiment is to send
 * a data frame to the OBC. When this function is called, the experiment should
 * copy over the correct amount of data into the buffer pointer to by buf.
 *
 * Example: Sending a sequence of 1300 bytes to the OBC (and MTU = 500)
 *   - If len = 500 and offset = 0, then bytes 0 to 499 in the sequence should
 *     be copied into the buffer pointed to by buf.
 *   - If len = 300 and offset = 1000, then bytes 1000 to 1299 in the sequence
 *     should be copied into the buffer pointed to by buf.
 *
 * It is important to never delete data as soon as it has been copied over into
 * buf or to try to keep track of the offset outside of this function as MSP
 * may ask for the same data again if a frame got corrupted.
 *
 * msp_expsend_start() will always be called before this function. This
 * function will be called as part of the transaction that was last initiated
 * by an invocation of msp_expsend_start().
 */
void msp_expsend_data(unsigned char opcode, unsigned char *buf, unsigned long len, unsigned long offset);

/**
 * @brief Called when an OBC Request transaction has been completed
 *        successfully.
 * @param opcode The opcode of the transaction.
 *
 * The function which the MSP library will call at the end of a successful
 * transaction where the experiment was sending data to the OBC.
 *
 * msp_expsend_start() will always be called before this function. This
 * function will be called as part of the transaction that was last initiated
 * by an invocation of msp_expsend_start().
 */
void msp_expsend_complete(unsigned char opcode);

/**
 * @brief Called when an OBC Request transaction is aborted.
 * @param opcode The opcode of the transaction.
 * @param error An error code representing the reason why the transaction was
 *              aborted. The relevant error codes can be found in
 *              msp_exp_error.h.
 *
 * This function is called if a transaction where the experiment is sending
 * data to the OBC encountered an unrecoverable error and must be aborted.
 * After aborting this transaction, the experiment should be ready to engage in
 * new transactions with the OBC.
 *
 * msp_expsend_start() will always be called before this function. This
 * function will be called as part of the transaction that was last initiated
 * by an invocation of msp_expsend_start().
 */
void msp_expsend_error(unsigned char opcode, int error);


/**
 * @brief Called at the start of an OBC Send transaction.
 * @param opcode The opcode of the transaction (As determined by the OBC.)
 * @param len The number of bytes that the OBC will send to the experiment in
 *            the transaction.
 *
 * The function which the MSP library will call at the start of a transaction
 * when the experiment will receive data from the OBC.
 *
 * OBS: This function will not be called if the OBC is sending a system
 * command. Instead, the function msp_exprecv_syscommand() will be called for
 * those opcodes.
 */
void msp_exprecv_start(unsigned char opcode, unsigned long len);

/**
 * @brief Called when data should be extracted from a data frame in an OBC Send
 *        transaction.
 * @param opcode The opcode of the transaction.
 * @param buf Pointer to the data field in the data frame.
 * @param len Length of the data field in the data frame.
 * @param offset The offset of the data in this data frame to the data in the
 *               sequence which the OBC is sending in the transaction.
 *               Example: If we have previously received two data frames with
 *               30 bytes each in the transaction, then the offset should be
 *               set to 60 if no errors has occurred during the transaction.
 *
 * The function which the MSP library will call when the experiment has
 * received a data frame from the OBC. The data from this data frame (pointed
 * to by buf) should be copied over into an area of storage or a buffer. The
 * parameter len determines how many bytes that should be copied and the offset
 * parameter determines where in the buffer/storage the bytes should be copied
 * into.
 *
 * Example: Receiving a sequence of 950 bytes from the OBC (and MTU = 500)
 *   - If len = 500 and offset = 0, then bytes 0 to 499 in the buffer pointer
 *     to by buf should be copied into the buffer/storage starting at byte 0.
 *   - If len = 450 and offset = 500, then bytes 0 to 449 in the buffer
 *     pointed to by buf should be copied over into the buffer/storage starting
 *     at byte 500. (I.e. storage[i + offset] = buf[i] for i = 0 to 449)
 *
 * It is important to never keep track of the offset outside of this function
 * as the OBC may retransmit the exact same data if a frame got corrupted.
 *
 * msp_exprecv_start() will always be called before this function. This
 * function will be called as part of the transaction that was last initiated
 * by an invocation of msp_exprecv_start().
 */
void msp_exprecv_data(unsigned char opcode, const unsigned char *buf, unsigned long len, unsigned long offset);

/**
 * @brief Called when an OBC Send transaction has been completed successfully.
 * @param opcode The opcode of the transaction.
 *
 * The function which the MSP library will call at the end of a transaction
 * where the experiment was receiving data from the OBC. When this is called,
 * the sequence of data that has been extracted from the msp_exprecv_data()
 * function during the transaction should be considered as the complete
 * sequence of data sent from the OBC.
 *
 * msp_exprecv_start() will always be called before this function. This
 * function will be called as part of the transaction that was last initiated
 * by an invocation of msp_exprecv_start().
 */
void msp_exprecv_complete(unsigned char opcode);

/**
 * @brief Called when an OBC Send transaction is aborted.
 * @param opcode The opcode of the transaction.
 * @param error An error code representing the reason why the transaction was
 *              aborted. The relevant error codes can be found in
 *              msp_exp_error.h.
 *
 * This function is called if a transaction where the experiment is receiving
 * data from the OBC has encountered an error and must be aborted. After
 * aborting this transaction, the experiment should be ready to engage in new
 * transactions with the OBC.
 *
 * msp_exprecv_start() will always be called before this function. This
 * function will be called as part of the transaction that was last initiated
 * by an invocation of msp_exprecv_start().
 */
void msp_exprecv_error(unsigned char opcode, int error);


/**
 * @brief Called when a system command is received from the OBC.
 * @param opcode The opcode of the system command.
 *
 * This function is called whenever a system command is received from the OBC.
 * In this case, no other "msp_exprecv" function is called. See the MSP
 * specification for a list of all system commands.
 */
void msp_exprecv_syscommand(unsigned char opcode);

#endif /* MSP_EXP_HANDLER_H */
