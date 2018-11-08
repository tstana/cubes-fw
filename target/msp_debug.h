/*
 * msp_debug.h
 * Author: John Wikman
 *
 * Defines debug functions for MSP.
 */

#ifndef MSP_EXP_DEBUG_H
#define MSP_EXP_DEBUG_H

#ifdef MSP_DEBUG
#include <stdio.h>
#define msp_debug(msg) printf("(MSP debug %s:%d) %s\n",__FILE__,__LINE__,(msg))
#define msp_debug_int(msg,i) printf("(MSP debug %s:%d) %s%d\n",__FILE__,__LINE__,(msg),(i))
#define msp_debug_hex(msg,h) printf("(MSP debug %s:%d) %s%X\n",__FILE__,__LINE__,(msg),(h))
#else
#define msp_debug(msg)
#define msp_debug_int(msg,i)
#define msp_debug_hex(msg,h)
#endif

#endif
