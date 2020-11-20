/*
 * msp_exp_handler.c
 *
 *  Created on: 26 nov. 2018
 *      Author: Marcus Persson
 */

#include "msp_i2c.h"
#include "msp_exp.h"

#include "../mem_mgmt/mem_mgmt.h"

#include "../hvps/hvps_c11204-02.h"

#include "../firmware/drivers/citiroc/citiroc.h"
#include "../firmware/drivers/cubes_timekeeping/cubes_timekeeping.h"
#include "../firmware/drivers/citiroc/citiroc.h"
#include "../firmware/drivers/mss_timer/mss_timer.h"


static uint8_t *send_data;
static unsigned char send_data_payload[25000]="";
static unsigned char send_data_hk[64] = "";

#define RECV_MAXLEN 	256
static unsigned char recv_data[RECV_MAXLEN] = "";
static unsigned long recv_length;

unsigned int has_send = 0;
static unsigned int has_send_error = 0;
static unsigned int has_send_errorcode = 0;
unsigned int has_recv = 0;
static unsigned int has_recv_error = 0;
static unsigned int has_recv_errorcode = 0;
unsigned int has_syscommand = 0;
static uint8_t comp_date[70];

static uint8_t daq_dur;


/*
 *------------------------------------------------------------------------------
 * Experiment Send Callbacks
 *------------------------------------------------------------------------------
 */
/**
 * @brief MSP callback at start of experiment send
 *
 * This function is called by the MSP API right after a REQ-type command was
 * received over MSP from the OBC. The experiment will need to send data.
 *
 * The function is used to prepare the data to send to the OBC, based on the
 * REQ op-code received.
 *
 * @param opcode[in]  The REQ opcode that was received from the OBC
 * @param len[out]    The length of data CUBES has to send to the OBC
 */
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
		/* HCRs */
		uint32_t count = 0;
		count = citiroc_hcr_get(0);
		to_bigendian32(send_data_hk, count);
		count = citiroc_hcr_get(16);
		to_bigendian32(send_data_hk+4, count);
		count = citiroc_hcr_get(31);
		to_bigendian32(send_data_hk+8, count);
		/* OR32 is "channel 32" */
		count = citiroc_hcr_get(32);
		to_bigendian32(send_data_hk+12, count);

		/* HVPS HK */
		sprintf(((char*)send_data_hk)+16, "%04X", hvps_get_voltage());
		sprintf(((char*)send_data_hk)+20, "%04X", hvps_get_current());
		sprintf(((char*)send_data_hk)+24, "%04X", hvps_get_temp());

		/* Re-use `count` variable for reading reset counters */
		count = nvm_reset_counter_read();
		to_bigendian32(send_data_hk+28, count);
		count = hvps_get_cmd_counter(HVPS_CMDS_SENT);
		send_data_hk[32] = (unsigned char) (count >> 8)  & 0xff;
		send_data_hk[33] = (unsigned char) (count >> 0)  & 0xff;
		count = hvps_get_cmd_counter(HVPS_CMDS_ACKED);
		send_data_hk[34] = (unsigned char) (count >> 8)  & 0xff;
		send_data_hk[35] = (unsigned char) (count >> 0)  & 0xff;
		count = hvps_get_cmd_counter(HVPS_CMDS_FAILED);
		send_data_hk[36] = (unsigned char) (count >> 8)  & 0xff;
		send_data_hk[37] = (unsigned char) (count >> 0)  & 0xff;

		send_data = (uint8_t *)send_data_hk;
		*len = 38;
	}
	else if(opcode == MSP_OP_REQ_CUBES_ID)
	{
		sprintf((char*)comp_date, "%s %s",__DATE__, __TIME__);
		uint32_t temp = citiroc_read_id();
		to_bigendian32(comp_date+21, temp);
		send_data = (uint8_t *)comp_date;
		*len = 25;
	}
	else
		*len = 0;
}

/**
 * @brief MSP callback before each data frame while experiment is sending data
 *
 * This function is called by the MSP API when a data block is to be sent
 * over MSP to the OBC. It is used by the experiment to fill up the frame with
 * data.
 *
 * @param opcode      The REQ opcode that was received from the OBC
 * @param buf[out]    The data buffer to be written to the frame
 * @param len         The length of data that should be inserted into `buf`.
 * @param offset      The offset at which data should be inserted into `buf`.
 *                    This is the number of bytes already sent by the experiment
 *                    to the OBC.
 */
void msp_expsend_data(unsigned char opcode,
                      unsigned char *buf,
                      unsigned long len,
                      unsigned long offset)
{
	for(unsigned long i = 0; i<len; i++){
		buf[i] = send_data[offset+i];
	}
}

/**
 * @brief MSP callback when experiment data sending completed (successfully)
 *
 * This function is called by the MSP API when experiment data sending has
 * proceeded successfully.
 *
 * @param opcode The opcode for the transaction that succeeded
 */
void msp_expsend_complete(unsigned char opcode)
{
	has_send = opcode;
	if(opcode == MSP_OP_REQ_PAYLOAD)
		memset(send_data_payload, '\0', sizeof(send_data_payload));
}

/**
 * @brief MSP callback when experiment send transaction failed
 *
 * This function is called when an MSP transaction had to be aborted by the OBC
 * due to an unrecoverable error.
 *
 * @param opcode The opcode for the transaction which failed
 * @param error  The error code, defined in `msp_exp_error.h`
 */
void msp_expsend_error(unsigned char opcode, int error)
{
	has_send_error = opcode;
	has_send_errorcode = error;
}


/*
 *------------------------------------------------------------------------------
 * Experiment Receive Callbacks
 *------------------------------------------------------------------------------
 */
/**
 * @brief MSP callback at start of experiment reception
 *
 * This function is called by the MSP API before the OBC sends data to the
 * experiment.
 *
 * @param opcode The SEND opcode for the upcoming transaction
 * @param len    The length of data to be sent by the OBC to the experiment
 */
void msp_exprecv_start(unsigned char opcode, unsigned long len)
{
	recv_length = len;
	memset(recv_data, '\0', sizeof(recv_data));
}


/**
 * @brief MSP callback after each data frame during experiment reception
 *
 * This function is called by the MSP API once a new batch of data has been
 * received from the OBC.
 *
 * @param opcode The SEND opcode for the on-going transaction
 * @param buf    Data received from the OBC
 * @param len    The length of data received in this data frame from the OBC
 * @param offset The number of data bytes already sent by the OBC
 *               Note that this number can be the same as in a previous data
 *               frame, if the frame was for some reason not received correctly
 *               by the OBC.
 */
void msp_exprecv_data(unsigned char opcode,
                      const unsigned char *buf,
                      unsigned long len,
                      unsigned long offset)
{
	for(unsigned long i=0; i<len; i++){
		if((i+offset) < RECV_MAXLEN){
			recv_data[i+offset] = buf[i];
		}
		else
			break;
	}
}

/**
 * @brief MSP callback for when the transaction completed successfully
 *
 * This function is used by CUBES to apply the data received as part of the
 * transaction. The `opcode` parameter dictates where the received data is to be
 * applied.
 *
 * @param opcode The SEND opcode for the transaction that completed successfully
 */
void msp_exprecv_complete(unsigned char opcode)
{
	switch (opcode) {
		case MSP_OP_SEND_TIME:
			cubes_set_time((recv_data[0] << 24) |
						   (recv_data[1] << 16) |
						   (recv_data[2] <<  8) |
						   (recv_data[3]));
			break;

		case MSP_OP_SEND_CUBES_HVPS_CONF:
		{
			uint8_t turn_on = recv_data[0] & 0x01;
			uint8_t reset = (uint8_t)recv_data[0] & 0x02;

			if (turn_on && !hvps_is_on())
				hvps_turn_on();
			else if (!turn_on && hvps_is_on())
				hvps_turn_off();
			if(reset && hvps_is_on())
				hvps_reset();

			/*
			 * Apply temperature correction factor if the command was not a
			 * "turn off" or a "reset"...
			 */
			if (turn_on && !reset) {
				struct hvps_temp_corr_factor f;

				f.dtp1 = (((uint16_t)recv_data[1]) << 8) |
				          ((uint16_t)recv_data[2]);
				f.dtp2 = (((uint16_t)recv_data[3]) << 8) |
				          ((uint16_t)recv_data[4]);
				f.dt1 = (((uint16_t)recv_data[5]) << 8) |
				         ((uint16_t)recv_data[6]);
				f.dt2 = (((uint16_t)recv_data[7]) << 8) |
				         ((uint16_t)recv_data[8]);
				f.vb = (((uint16_t)recv_data[ 9]) << 8) |
				        ((uint16_t)recv_data[10]);
				f.tb = (((uint16_t)recv_data[11]) << 8) |
				        ((uint16_t)recv_data[12]);

				hvps_set_temp_corr_factor(&f);
				hvps_temp_compens_en();
			}
			break;
		}

		case MSP_OP_SEND_CUBES_HVPS_TMP_VOLT:
		{
			uint8_t turn_on = recv_data[0] & 0x01;
			uint8_t reset = recv_data[0] & 0x02;

			if (turn_on && !hvps_is_on())
				hvps_turn_on();
			else if (!turn_on && hvps_is_on())
				hvps_turn_off();
			if(reset && hvps_is_on())
				hvps_reset();

			if (turn_on && !reset)
				hvps_set_temporary_voltage((((uint16_t)recv_data[1]) << 8) |
				                            ((uint16_t)recv_data[2]));

			break;
		}

		case MSP_OP_SEND_CUBES_CITI_CONF:
			mem_ram_write(RAM_CITI_CONF, recv_data);
			citiroc_send_slow_control();
			break;

		case MSP_OP_SEND_CUBES_PROB_CONF:
			mem_ram_write(RAM_CITI_PROBE, recv_data);
			citiroc_send_probes();
			break;

		case MSP_OP_SEND_READ_REG_DEBUG:
		{
			citiroc_rrd(recv_data[0] & 0x01, (recv_data[0] & 0x3e)>>1);
			break;
		}

		case MSP_OP_SEND_CUBES_DAQ_DUR:
			daq_dur = recv_data[0];
			citiroc_daq_set_dur(daq_dur);
			break;

		case MSP_OP_SEND_CUBES_GATEWARE_CONF:
		{
			uint8_t resetvalue = recv_data[0];
			if (resetvalue & 0b00000001)
				nvm_reset_counter_reset();
			if (resetvalue & 0b00000010)
				citiroc_hcr_reset();
			if (resetvalue & 0b00000100)
				citiroc_histo_reset();
			if (resetvalue & 0b00001000)
				citiroc_psc_reset();
			if (resetvalue & 0b00010000)
				citiroc_sr_reset();
			if (resetvalue & 0b00100000)
				citiroc_pa_reset();
			if (resetvalue & 0b01000000)
				citiroc_trigs_reset();
			if (resetvalue & 0b10000000)
				citiroc_read_reg_reset();
			break;
		}
	}

	has_recv=opcode;
}

/**
 * @brief MSP callback for when the experiment receive transaction failed
 *
 * @param opcode The opcode for the transaction that failed
 * @param error  The error code, defined in `msp_exp_error.h`
 */
void msp_exprecv_error(unsigned char opcode, int error)
{
	has_recv_error=opcode;
	has_recv_errorcode = error;
}

/*
 *------------------------------------------------------------------------------
 * System Command Callbacks
 *------------------------------------------------------------------------------
 */
/**
 * @brief MSP callback for when a system command has been received
 *
 * @param opcode The opcode for the received system command
 */
void msp_exprecv_syscommand(unsigned char opcode)
{
	switch(opcode) {
		case MSP_OP_ACTIVE:
			hvps_turn_on();
			break;
		case MSP_OP_SLEEP:
			hvps_turn_off();
			citiroc_daq_stop();
			break;
		case MSP_OP_POWER_OFF:
			hvps_turn_off();
			citiroc_daq_stop();
			msp_save_seqflags();
			break;
		case MSP_OP_CUBES_DAQ_START:
			citiroc_hcr_reset();
			citiroc_histo_reset();
			citiroc_daq_set_hvps_temp(hvps_get_temp());
			citiroc_daq_set_hvps_volt(hvps_get_voltage());
			citiroc_daq_set_hvps_curr(hvps_get_current());
			MSS_TIM2_load_immediate(((daq_dur-1)*100000000)&0xFFFFFFFF);
			MSS_TIM2_start();
			citiroc_daq_start();
			break;
		case MSP_OP_CUBES_DAQ_STOP:
			citiroc_daq_stop();
			break;
	}

	has_syscommand = opcode;
}


void Timer2_IRQHandler(void)
{
	citiroc_daq_set_hvps_temp(hvps_get_temp());
	citiroc_daq_set_hvps_volt(hvps_get_voltage());
	citiroc_daq_set_hvps_curr(hvps_get_current());
	MSS_TIM2_clear_irq();
}
