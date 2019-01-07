/*
 * UART_comm_int.h
 *
 *  Created on: 6 nov. 2018
 *      Author: Marcus Persson
 */

#ifndef UART_COMM_INT_H_
#define UART_COMM_INT_H_

#include "../../cubes-fw-libero/drivers/mss_uart/mss_uart.h"

/*
 * Initalizes all interrupts and timestamp timer
 * Input: None
 * Output: None
 *
 */
void init_uart(char* memory);

/*
 * uart1_rx_handler
 * Interrupt for handling UART communication with HVPS side.
 * Input: UART pointer
 * Output: None
 *
 */
void uart1_rx_handler(mss_uart_instance_t * this_uart);


/*
 * Timer1_IRQHandler
 * Interrupt for sending UART commands for status updates from HVPS
 * Input:  None
 * Output: None
 *
 */
void Timer1_IRQHandler(void);

#endif /* UART_COMM_INT_H_ */
