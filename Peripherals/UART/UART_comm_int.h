/*
 * UART_comm_int.h
 *
 *  Created on: 6 nov. 2018
 *      Author: Marcus Persson
 */

#ifndef UART_COMM_INT_H_
#define UART_COMM_INT_H_

#include "../../firmware/drivers/mss_uart/mss_uart.h"

/*
 * Initalizes all interrupts and timestamp timer
 * Input: None
 * Output: None
 *
 */
void initUART(void);

/*
 * uart1_rx_handler
 * Interrupt for handling UART communication with HVPS side.
 * Input: UART pointer
 * Output: None
 *
 */
void uart1_rx_handler(mss_uart_instance_t * this_uart);


/*
 * uart0_rx_handler
 * Interrupt for handling UART communication with SmartFusion2 side
 * Input: UART pointer
 * Output: None
 *
 */
void uart0_rx_handler(mss_uart_instance_t * this_uart);

#endif /* UART_COMM_INT_H_ */
