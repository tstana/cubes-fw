/*
 * msp_opcodes.h
 * Author: John Wikman
 *
 * Defines all the opcodes in MSP.
 */

#ifndef MSP_OPCODES_H
#define MSP_OPCODES_H

/* MSP Control Flow */
#define MSP_OP_NULL        0x00
#define MSP_OP_DATA_FRAME  0x01
#define MSP_OP_F_ACK       0x02
#define MSP_OP_T_ACK       0x03
#define MSP_OP_EXP_SEND    0x04
#define MSP_OP_EXP_BUSY    0x05

/* System Commands */
#define MSP_OP_ACTIVE      0x10
#define MSP_OP_SLEEP       0x11
#define MSP_OP_POWER_OFF   0x12

/* Standard OBC Request */
#define MSP_OP_REQ_PAYLOAD 0x20
#define MSP_OP_REQ_HK      0x21
#define MSP_OP_REQ_PUS     0x22

/* Standard OBC Read */
#define MSP_OP_SEND_TIME   0x30
#define MSP_OP_SEND_PUS    0x31

/* CUBES custom opcodes */
#define MSP_OP_CUBES_DAQ_START	0x51
#define MSP_OP_CUBES_DAQ_STOP	0x52

#define MSP_OP_REQ_CUBES_ID		0x61

#define MSP_OP_SEND_CUBES_HVPS_CONF				0x71
#define MSP_OP_SEND_CUBES_CITI_CONF     		0x72
#define MSP_OP_SEND_CUBES_PROB_CONF     		0x73
#define MSP_OP_SEND_CUBES_DAQ_CONF 				0x74
#define MSP_OP_SEND_CUBES_HVPS_TMP_VOLT			0x75
#define MSP_OP_SEND_READ_REG_DEBUG				0x76
#define MSP_OP_SEND_CUBES_GATEWARE_CONF			0x77
#define MSP_OP_SEND_CUBES_CALIB_PULSE_CONF		0x78

/* Values for determining opcode type */
#define MSP_OP_TYPE_CTRL 0x00
#define MSP_OP_TYPE_SYS  0x10
#define MSP_OP_TYPE_REQ  0x20
#define MSP_OP_TYPE_SEND 0x30

#define MSP_OP_TYPE(opcode) ((opcode) & 0x30)

#define MSP_OP_IS_CUSTOM(opcode) (((opcode) & 0x70) >= 0x50)

#endif
