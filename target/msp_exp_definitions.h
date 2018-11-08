/*
 * msp_exp_definitions.h
 * Author: John Wikman
 *
 * Contains constant definitions that MSP uses during communication. Each
 * constant is described here:
 *  - MSP_ADDR: The address of the experiment.
 *  - MSP_EXP_MTU: The maximum transmission unit agreed upon by the experiment
 *             and the OBC. This specifies the maximum amount of data that can
 *             fit inside of the data field of a frame.
 *  - MSP_MAX_FRAME_SIZE: The maximum size a received MSP frame can have. Use
 *                        this constant to determine the size of the receive
 *                        send buffer.
 *
 */

#ifndef MSP_EXP_DEFINITIONS_H
#define MSP_EXP_DEFINITIONS_H

/* Import MSP_EXP_ADDR and MSP_EXP_MTU from the configuration file */
#include "msp_configuration.h"

#ifndef MSP_EXP_ADDR
#error MSP_EXP_ADDR not set
#endif

#ifndef MSP_EXP_MTU
#error MSP_EXP_MTU not set
#else
#define MSP_EXP_MAX_FRAME_SIZE (((MSP_EXP_MTU) + 5) > 9 ? ((MSP_EXP_MTU) + 5) : 9)
#endif


#endif
