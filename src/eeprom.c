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
 *  \file eeprom.c
 *  \brief EEPROM support
 */

// TODO: see if we can do buffering and use the interrupt to transmit data


/*===== GLOBAL INCLUDES ======================================================*/

#include <avr/io.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/eeprom.h"

#if ARTX_ENABLE_EEPROM

/*===== DEFINES ==============================================================*/

/*===== TYPEDEFS =============================================================*/

/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

/*===== EXTERNAL VARIABLES ===================================================*/

/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

/*===== STATIC FUNCTIONS =====================================================*/

/*===== FUNCTIONS ============================================================*/

/**
 *  Transmit a single byte
 *
 *  Transmit a single byte via the SPI interface.
 *  The routine blocks until the byte is transmitted.
 *
 *  \param data                  Byte to transmit.
 */

uint8_t ARTX_eeprom_read_byte(uint16_t address)
{
  ARTX_disable_int();

  /* Wait for completion of previous write */
  while(EECR & (1 << EEWE))
  {
    (void) 0;
  }

  EEAR = address;

  EECR |= 1 << EERE;

  uint8_t rv = EEDR;

  ARTX_enable_int();

  return rv;
}

void ARTX_eeprom_write_byte(uint16_t address, uint8_t data)
{
  /* Wait for completion of previous write */
  while(EECR & (1 << EEWE))
  {
    (void) 0;
  }

  EEAR = address;
  EEDR = data;

  EECR |= 1 << EEMWE;
  EECR |= 1 << EEWE;
}

void ARTX_eeprom_read(uint16_t address, uint8_t *data)
{
}

void ARTX_eeprom_write(uint16_t address, const uint8_t *data)
{
}

#endif
