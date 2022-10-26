/*
 *==============================================================================
 * CUBES - Timekeeping
 *
 * Header file
 *==============================================================================
 *
 * author: Theodor Stana (stana@kth.se)
 *
 * date of creation: 2019-02-13
 *
 * description:
 *
 * 	This file contains functions to get and set the UTC time in CUBES gateware.
 *
 *==============================================================================
 * GNU LESSER GENERAL PUBLIC LICENSE
 *==============================================================================
 * This source file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version. This source is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details. You should have
 * received a copy of the GNU Lesser General Public License along with this
 * source; if not, download it from http://www.gnu.org/licenses/lgpl-2.1.html
 *==============================================================================
 */

#ifndef _CUBES_TIMEKEEPING_H_
#define _CUBES_TIMEKEEPING_H_

#include <stdint.h>
#include "../../cubes_hw_platform.h"

#define CUBES_TIME_REG    (*(volatile uint32_t *)CUBES_TIMEKEEPING)

void        cubes_set_time(uint32_t newtime);
uint32_t    cubes_get_time(void);

#endif /* _CUBES_TIMEKEEPING_H_ */
