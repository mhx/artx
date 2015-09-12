#ifndef artx_EEPROM_H_
#define artx_EEPROM_H_

/*******************************************************************************
*
* ARTX EEPROM support
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
 *  \file artx/eeprom.h
 *  \brief EEPROM support
 */

#include <avr/pgmspace.h>

#include "artx/artx.h"

#if ARTX_ENABLE_EEPROM

uint8_t ARTX_eeprom_read_byte(uint16_t address);
void ARTX_eeprom_write_byte(uint16_t address, uint8_t data);
void ARTX_eeprom_read(uint16_t address, uint8_t *data);
void ARTX_eeprom_write(uint16_t address, const uint8_t *data);

#endif

#endif
