/*
 * uart_comm_func.h
 *
 *  Created on: 27 nov. 2018
 *      Author: Zethian
 */

#ifndef PERIPHERALS_UART_UART_COMM_FUNC_H_
#define PERIPHERALS_UART_UART_COMM_FUNC_H_

/*
 * Sends string to hvps with some boot settings (not needed in later documentation)
 * Input: None
 * Output: None
 *
 */

void start_hvps(void);

/*
 * voltage_check
 * Checks input voltage level to see that it's within MPPC specifications
 * Input: Command with HEX voltage value.
 * Output: 0 if passed, -1 if voltage too high
 */

int voltage_check(uint8_t cmd[28]);

/*
 * checksum_control
 * Controls Checksum value for HVPS response
 * Input: Command for HVPS
 * Output: 0 if passed, -1 if not passed. Int values.
 */

int checksum_control(uint8_t cmd[28]);
/*
 * Function takes pointer to destination array and commandline received from terminal, and formats it to readable for hvps
 * Input: Char array pointer, char array command
 * Output: None
 *
 */

void getarray(uint8_t *array, uint8_t cmd[28]);




/*
 * hvps_send_voltage()
 * Takes incoming voltage command and transforms it into hex and formats the HST command to uart.
 * Input: A string literal with voltage in decimal form
 * Output: Return -1 for fail, 0 for pass.
 */

int hvps_send_voltage(char command[]);

/*
 * hvps_turn_on()
 * Command for turning on the hvps
 *
 */
void hvps_turn_on(void);

/*
 * hvps_turn_off()
 * Command for turning the hvps off
 */

void hvps_turn_off(void);

#endif /* PERIPHERALS_UART_UART_COMM_FUNC_H_ */
