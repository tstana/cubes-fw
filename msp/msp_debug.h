/**
 * @file      msp_debug.h
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Debug functionalities for MSP.
 *
 * @details
 * Defines debug functinalities for MSP. If MSP_DEBUG is defined, then printf
 * will be used to print the debug messages. Otherwise, the debug functions
 * will have no effect. Ideally, MSP_DEBUG should be defined at compile time
 * to avoid accidental debug prints in deployment.
 */

#ifndef MSP_DEBUG_H
#define MSP_DEBUG_H

#ifndef MSP_DEBUG
/**
 * @brief Prints a debug message.
 * @param msg The message to be printed.
 */
#define msp_debug(msg)

/**
 * @brief Prints a debug message and an integer.
 * @param msg The message to be printed.
 * @param i The integer to by printed after the message.
 */
#define msp_debug_int(msg,i)

/**
 * @brief Prints a debug message followed by a hexadecimal number.
 * @param msg The message to be printed.
 * @param h The hexadecimal number to be printed after the message.
 */
#define msp_debug_hex(msg,h)
#else
#include <stdio.h>
#define msp_debug(msg) printf("(MSP debug %s:%d) %s\n",__FILE__,__LINE__,(msg))
#define msp_debug_int(msg,i) printf("(MSP debug %s:%d) %s%d\n",__FILE__,__LINE__,(msg),(i))
#define msp_debug_hex(msg,h) printf("(MSP debug %s:%d) %s%X\n",__FILE__,__LINE__,(msg),(h))
#endif

#endif /* MSP_DEBUG_H */
