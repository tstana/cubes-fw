/*
 * I2C_comm_func.c
 *
 *  Created on: 29 okt. 2018
 *      Author: Marcus Persson
 */

#include "../UART/uart_comm_int.h"
#include "i2c_comm_includes.h"




/* TODO: cannot be implemented while external storage is not defined.
 int cubes_to_storage(unsigned char *data, unsigned long length){

}
int cubes_from_storage(unsigned char *data, unsigned long length){

}

void initStorage(void);{ /* external shall be a pointer to start of external memory */
	/*buffer_t __attribute__((section("EXT_MEM_CUBES"))) cubes_storage[5000];

}*/
