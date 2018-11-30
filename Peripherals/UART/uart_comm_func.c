/*
 * uart_comm_func.c
 *
 *  Created on: 27 nov. 2018
 *      Author: Marcus Persson
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../../firmware/drivers/mss_uart/mss_uart.h"
#include "uart_comm_func.h"



uint8_t chkstr[2];
uint8_t send[32];
uint16_t rec=0;

mss_uart_instance_t * const gp_my_uart1 = &g_mss_uart1;




void getarray(uint8_t *array, uint8_t cmd[28]){
	const uint8_t stx = 0x02;
	const uint8_t etx = 0x03;
	const uint8_t CR = 0x0D;
	uint16_t chksm=0x00;
	/* Memmove is used with offset for the adress because strcat did not give the proper format when sending it on to the HVPS */
	int cmdlen = strlen(cmd);
	memmove(array, &stx, 1);
	memmove(array+1, cmd, cmdlen);
	memmove(array+1+cmdlen, &etx, 1);
	for(int i = 0; array[i-1]!=0x03; i++){
		chksm+=array[i];
	}
	chksm = (chksm & 0xFF); /* Mask so only lower 2 bytes get sent */
	sprintf(chkstr, "%02X", chksm);
	memmove(array+2+cmdlen, chkstr, 2);
	memmove(array+4+cmdlen, &CR, 1);
	strcpy(cmd, "");

}


void start_hvps(void){
	char temp[50] = "";
	strcpy(temp, "HST03E803E800000000600000C8");
	getarray(send, temp); /*get required string from function */
	MSS_UART_polled_tx_string(gp_my_uart1, send);
	memset(send, 0, sizeof(send));
}

/*
 * voltageCheck
 * Reads received command to see that voltage is not over 55V (limit on MPPC)
 * Input: Command char array
 * Output: Return -1 as int if over 55V, return 0 otherwise
 *
 */
int voltage_check(uint8_t cmd[28]){
	uint8_t data[4] = "";
	uint16_t val = 0;
	if((cmd[0]=='H' && cmd[1]=='S' && cmd[2]=='T')) {
		for(int i=0; i<4; i++){
			data[i] = cmd[i+19];
		}
	}
	else if(cmd[0]=='H' && cmd[1]=='B' && cmd[2]=='V'){
		for(int i=0; i<4; i++){
			data[i] = cmd[i+3];
		}
	}
	val=strtol(data, NULL, 16);
	val=round(val*(1.812/pow(10, 3)));
	if(val > 55)
		return -1;
	else
		return 0;
}

/*
 * TODO: Behövs den?
 */
int checksum_control(uint8_t cmd[28]){
	uint16_t i=0;
	uint16_t sum = 0x00;
	uint8_t check[2] = "";
	while(cmd[i]!= 0x03){
		sum+=cmd[i];
		i++;
	}
	sum+=cmd[i];
	sum = (sum & 0xFF); /* Mask so only lower 2 bytes get sent */
	sprintf(check, "%02X", sum);
	if(cmd[++i] == check[0] && cmd[++i] == check[1]){
		return 0;
	}
	else
		return -1;
}


int hvps_send_voltage(char command[]){
	char HST[30]="HST03E803E800000000600000C8"; /* Standard input, ~52V, no temp correction */
	long voltage = strtol(command, NULL, 10); /* Get integer format of input voltage */
	char hexvolt[4];
	voltage = voltage * 1.812/pow(10, 3);
	sprintf(hexvolt, "%04X", voltage);
	for (int i=0; i<4; i++){ /* Move voltage into temperature correction factor */
		HST[19+i]=hexvolt[i];
	}
	if(voltageCheck(HST) == -1)
		return -1;
	getarray(send, HST); /* Format string to UART and send it on */
	MSS_UART_polled_tx_string(gp_my_uart1, send);
	memset(send, 0, sizeof(send));
	return 0;
}

void hvps_turn_on(void){
	char HON[] = "HON";
	getarray(send, HON);
	MSS_UART_polled_tx_string(gp_my_uart1, HON);
	memset(send, 0, sizeof(send));
}

void hvps_turn_off(void){
	char HOF[] = "HOF";
	getarray(send, HOF);
	MSS_UART_polled_tx_string(gp_my_uart1, HOF);
	memset(send, 0, sizeof(send));
}

/*int HVPS_read_voltage(void){
	char HGV[20] = "";
	getarray(HGV, "HGV");
	MSS_UART_polled_tx_string(gp_my_uart1, HGV);
}*/
