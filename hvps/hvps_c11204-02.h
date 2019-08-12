/**
 * hvps_c11204-02.h
 *
 *  Created on: 10 jan. 2019
 *      Author: Marcus Persson
 */

#ifndef HM_HVPS_H_
#define HM_HVPS_H_

/**
 * Initalizes all interrupts and timestamp timer
 * @param Memory adress of configuration data
 * Output: None
 *
 */
void hvps_init(void);

/**
 * hvps_set_voltage()
 * Takes incoming voltage command and transforms it into hex and formats the HST
 * command to UART.
 * @param A string literal with voltage in decimal form
 * @return Return -1 for fail, 0 for pass.
 */
int hvps_set_temp_corr_factor(uint8_t* command);

/**
 * hvps_set_temporary_voltage()
 *
 * @brief Sets a temporary voltage at the HVPS output using the "HBV" command.
 *
 * @param Two-byte value corresponding to the ASCII value sent to the HVPS
 *        module. For example, for an ASCII setting of "3AEF" (four bytes), the
 *        corresponding two-byte value (the passed parameter value) is 0x3AEF.
 * @return -1 for fail, 0 for pass.
 */
int hvps_set_temporary_voltage(uint16_t v);

/**
 * hvps_send_cmd(char* cmd)
 */

void hvps_send_cmd(uint8_t cmd[]);

/**
 * hvps_is_on(void)
 *
 * @brief Get the latest obtained status from the HVPS.
 *
 * @return '1' if HVPS output voltage is on, '0' otherwise.
 */
uint8_t hvps_is_on(void);


/**
 * hvps_tempr_write
 *
 * Writes the current temperature located in the housekeeping values
 * to the first 16 bits of CITIROC->TEMPR
 */

void hvps_tempr_write(void);


#endif /* HM_HVPS_H_ */
