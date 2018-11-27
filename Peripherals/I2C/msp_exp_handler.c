/*
 * msp_exp_handler.c
 *
 *  Created on: 26 nov. 2018
 *      Author: Marcus Persson
 */

#include "../../msp/msp_exp.h"
#include "../UART/uart_comm_int.h"

static unsigned char *send_data;
static unsigned char send_data_payload[30] = "This is data, important data";
volatile unsigned char send_data_hk[50] = "This is just some boring numbers about status";
static unsigned char send_data_pus[60] = "Packet Utilization Standard Packet Utilization Standard PUS";

static unsigned char recv_data[501];
static unsigned long recv_maxlen = 500;
static unsigned long recv_length;

static unsigned int has_send = 0;
static unsigned int has_send_error = 0;
static unsigned int has_send_errorcode = 0;
static unsigned int has_recv = 0;
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
	else if(opcode == MSP_OP_REQ_PUS){ /* TBD: is this needed, or can we ignore? */
		send_data = send_data_pus;
		*len = sizeof(send_data_pus);
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
	MSS_UART_polled_tx_string(&g_mss_uart0, recv_data);
}

void msp_exprecv_error(unsigned char opcode, int error){
	has_recv_error=opcode;
	has_recv_errorcode = error;
}

void msp_exprecv_syscommand(unsigned char opcode){
	has_syscommand = opcode;
}
