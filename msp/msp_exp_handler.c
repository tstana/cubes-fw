/*
 * msp_exp_handler.c
 *
 *  Created on: 26 nov. 2018
 *      Author: Marcus Persson
 */

#include "msp_i2c.h"
#include "../msp/msp_exp.h"
#include "../mem_mgmt/mem_mgmt.h"
#include "../firmware/drivers/citiroc/citiroc.h"
#include "../hvps/hvps_c11204-02.h"

static uint8_t *send_data;
static unsigned char send_data_payload[25000]="";
static unsigned char send_data_hk[400] = "";

unsigned char recv_data[2000] = "";
static unsigned long recv_maxlen = 2000;
static unsigned long recv_length;

unsigned int has_send = 0;
static unsigned int has_send_error = 0;
static unsigned int has_send_errorcode = 0;
unsigned int has_recv = 0;
static unsigned int has_recv_error = 0;
static unsigned int has_recv_errorcode = 0;
unsigned int has_syscommand = 0;

volatile uint8_t hk_add_done = 0;



/* Prototype in msp_exp_handler.h */

void msp_expsend_start(unsigned char opcode, unsigned long *len)
{
	uint32_t *long_data = (uint32_t *)HISTO_RAM;
	if(opcode == MSP_OP_REQ_PAYLOAD && citiroc_daq_is_rdy())
	{
		/*
		 * Massage bin memory (16-bit, big-endian, organized into 2x 16-bit
		 * big-endian) into MSP data array; see mem-addressing.png for a visual
		 * description of this.
		 */
		for(int i=0; i<HISTO_LEN/4; i++) {
			send_data_payload[i*4+1] = long_data[i] & 0xFF;
			send_data_payload[i*4+0] = long_data[i]>>8 & 0xFF;
			send_data_payload[i*4+3] = long_data[i]>>16 & 0xFF;
			send_data_payload[i*4+2] = long_data[i]>>24 & 0xFF;
		}

		*len = HISTO_LEN;
		send_data = (uint8_t*) send_data_payload;
	}
	else if(opcode == MSP_OP_REQ_HK)
	{
		uint32_t hcr = 0;
		hcr = citiroc_hcr_get(0);
		to_bigendian32(send_data_hk, hcr);
		hcr = citiroc_hcr_get(16);
		to_bigendian32(send_data_hk+4, hcr);
		hcr = citiroc_hcr_get(31);
		to_bigendian32(send_data_hk+8, hcr);
		hcr = citiroc_hcr_get(21);
		to_bigendian32(send_data_hk+12, hcr);
		send_data = (uint8_t *)send_data_hk;
		*len = 28;
	}
	else
		*len = 0;
}

void msp_expsend_data(unsigned char opcode, unsigned char *buf, unsigned long len, unsigned long offset){
	for(unsigned long i = 0; i<len; i++){
		buf[i] = send_data[offset+i];
	}
}

void msp_expsend_complete(unsigned char opcode){
	has_send = opcode;
	if(opcode == MSP_OP_REQ_PAYLOAD)
		memset(send_data_payload, '\0', sizeof(send_data_payload));
	else if(opcode == MSP_OP_REQ_HK)
		memset(send_data_hk, '\0', sizeof(send_data_hk));
}
void msp_expsend_error(unsigned char opcode, int error){
	has_send_error = opcode;
	has_send_errorcode = error;
}

void msp_exprecv_start(unsigned char opcode, unsigned long len){
	recv_length = len;
	memset(recv_data, '\0', sizeof(recv_data));
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

void msp_add_hk(unsigned char *buff, unsigned long len, int offset){
	for (unsigned long i=0; i<len; i++){
		send_data_hk[i+offset] = buff[i];
	}
}

unsigned char* msp_get_recv(void){
	return (unsigned char*)recv_data;
}
