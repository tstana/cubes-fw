/*
 * msp_exp_handler.c
 *
 *  Created on: 26 nov. 2018
 *      Author: Marcus Persson
 */

#include "../../msp/msp_exp.h"
#include "../RTC/rtc.h"
#include "i2c_comm_includes.h"

static unsigned char *send_data;
static unsigned char send_data_payload[30] = "This is data, important data";
volatile unsigned char send_data_hk[50] = "This is just some boring numbers about status";

static unsigned char* recv_data;
static unsigned long recv_maxlen = 500;
static unsigned long recv_length;
unsigned char time_data[100] = "";

unsigned int has_send = 0;
static unsigned int has_send_error = 0;
static unsigned int has_send_errorcode = 0;
unsigned int has_recv = 0;
static unsigned int has_recv_error = 0;
static unsigned int has_recv_errorcode = 0;
static unsigned int has_syscommand = 0;

/* Prototype in msp_exp_handler.h */

void msp_expsend_start(unsigned char opcode, unsigned long *len){
	if(opcode == MSP_OP_REQ_PAYLOAD){
		send_data = send_data_payload; /* TODO:Change to payload data location */
		*len = sizeof(send_data_payload);
	}
	else if(opcode == MSP_OP_REQ_HK){
		send_data = (char*)send_data_hk; /* TODO:Change to housekeeping data location */
		*len = sizeof(send_data_hk);
	}
	else
		*len = 0;
}

void msp_expsend_data(unsigned char opcode, unsigned char *buf, unsigned long len, unsigned long offset){
	for(unsigned long i = 0; i<len; i++){
		buf[i] = send_data[offset+i];
	}
}

void msp_expsend_complete(unsigned char opcode){ /* TODO: get offset and clear data there? */
	has_send = opcode;
}
void msp_expsend_error(unsigned char opcode, int error){
	has_send_error = opcode;
	has_send_errorcode = error;
}

void msp_exprecv_start(unsigned char opcode, unsigned long len){
	recv_length = len;
	if(opcode==MSP_OP_SEND_TIME){
		*recv_data = &time_data;
	}
}
void msp_exprecv_data(unsigned char opcode, const unsigned char *buf, unsigned long len, unsigned long offset){
	for(unsigned long i=0; i<len; i++){
		if((i+offset) < recv_maxlen){
			recv_data[i+offset] = buf[i];
		}
		else
			break;
	}
}

void msp_exprecv_complete(unsigned char opcode){
	has_recv=opcode;
}

void msp_exprecv_error(unsigned char opcode, int error){
	has_recv_error=opcode;
	has_recv_errorcode = error;
}

void msp_exprecv_syscommand(unsigned char opcode){
	has_syscommand = opcode;
}
