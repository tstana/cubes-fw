/*
 *==============================================================================
 * CUBES - Timekeeping
 *
 * Source file
 *==============================================================================
 *
 * author: Theodor Stana (stana@kth.se)
 *
 * date of creation: 2019-02-14
 *
 * description:
 *
 *      TODO: Add description here...
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

#include "cubes_timekeeping.h"

/* TODO: Add return here, if error on AMBA bus? */
void cubes_set_time(uint32_t newtime)
{
	CUBES_TIME_REG = newtime;
}

uint32_t cubes_get_time(void)
{
	return CUBES_TIME_REG;
}

