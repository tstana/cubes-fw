/*
 * i2c_opcodes.h
 *
 *  Created on: 17 dec. 2018
 *      Author: Zethian
 */

#ifndef PERIPHERALS_I2C_I2C_OPCODES_H_
#define PERIPHERALS_I2C_I2C_OPCODES_H_

/* CUBES Citiroc communication 0x0* */
#define CITI_CONF 0x01

/* System commands 0x1* */
#define SYS_LOW 0x11
#define SYS_ON 0x12
#define SYS_OFF 0x13
#define SYS_RESET 0x14

/* CUBES HVPS communication 0x2* */
#define HVPS_ON 0x21
#define HVPS_OFF 0x22
#define HVPS_VOL 0x23
#define HVPS_TEMP 0x24 /* temperature correction on/off */
#define HVPS_CONF 0x25
/* CUBES Storage configuration 0x3* */

/* CUBES 0x4*  */

#endif /* PERIPHERALS_I2C_I2C_OPCODES_H_ */
