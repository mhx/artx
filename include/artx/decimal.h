#ifndef artx_DECIMAL_H_
#define artx_DECIMAL_H_

/*******************************************************************************
*
* ARTX decimal conversion functions
*
********************************************************************************
*
* ARTX - A realtime executive library for Atmel AVR microcontrollers
*
* Copyright (C) 2007-2015 Marcus Holland-Moritz.
*
* ARTX is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ARTX is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ARTX.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

/**
 *  \file artx/decimal.h
 *  \brief Decimal conversion functions
 */

#include <avr/io.h>

#include "artx/artx.h"

uint32_t ARTX_div10_u32(uint32_t n, uint8_t *p_r);
uint16_t ARTX_div10_u16(uint16_t n, uint8_t *p_r);
uint8_t ARTX_div10_u8(uint8_t n, uint8_t *p_r);

uint8_t ARTX_dec2bcd_u8(uint8_t dec);
uint16_t ARTX_dec2bcd_u16(uint16_t dec);
uint32_t ARTX_dec2bcd_u32(uint32_t dec);

uint8_t ARTX_bcd2dec_u8(uint8_t bcd);

#endif
