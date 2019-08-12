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
 * hvps_turn_on()
 * Command for turning on the hvps
 *
 */
void hvps_turn_on(void);

/**
 * hvps_turn_off()
 * Command for turning the hvps off
 */

void hvps_turn_off(void);

/**
 * hvps_get_voltage(void)
 *
 * Sends a command to retrieve voltage level from HVPS
 */

void hvps_get_voltage(void);

/**
 * hvps_get_current(void)
 *
 * Sends a command to retrieve current level from HVPS
 */
void hvps_get_current(void);

/**
 * hvps_get_temp(void)
 *
 * Sends a command to retrieve temperature from HVPS
 */
void hvps_get_temp(void);

/**
 * hvps_get_status(void)
 *
 * @brief Send a command to retrieve the status of the HVPS module.
 */
void hvps_get_status(void);

/**
 * hvps_is_on(void)
 *
 * @brief Get the latest obtained status from the HVPS.
 *
 * @return '1' if HVPS output voltage is on, '0' otherwise.
 */
uint8_t hvps_is_on(void);


/**
 * hvps_get_temp_corr_factor(void)
 *
 * Sends a command to retrieve temperature correction factor from HVPS.
 */
void hvps_get_temp_corr_factor(void);

/**
 * hvps_tempr_write
 *
 * Writes the current temperature located in the housekeeping values
 * to the first 16 bits of CITIROC->TEMPR
 */

void hvps_tempr_write(void);
/**
 * hvps_reset(void)
 *
 * Sends a command to reset the hvps output.
 */

void hvps_reset(void);

#endif /* HM_HVPS_H_ */
