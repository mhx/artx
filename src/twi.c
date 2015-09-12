/*******************************************************************************
*
* ARTX TWI/I2C interface
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
 *  \file spi.c
 *  \brief TWI/I2C interface
 */

// TODO: see if we can do buffering and use the interrupt to transmit data


/*===== GLOBAL INCLUDES ======================================================*/

#include <avr/io.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/twi.h"

#if ARTX_ENABLE_TWI

/*===== DEFINES ==============================================================*/

/*===== TYPEDEFS =============================================================*/

/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

/*===== EXTERNAL VARIABLES ===================================================*/

/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

/*===== STATIC FUNCTIONS =====================================================*/

/*===== FUNCTIONS ============================================================*/

uint8_t ARTX_twi_start(uint8_t addr, uint8_t write)
{
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  while (!(TWCR & (1<<TWINT)));
  return ARTX_twi_tx_byte((addr << 1) | (write ? 0 : 1));
}

void ARTX_twi_stop(void)
{
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

uint8_t ARTX_twi_tx_byte(uint8_t data)
{
  TWDR = data;
  TWCR = (1<<TWINT) | (1<<TWEN);
  while (!(TWCR & (1<<TWINT)));
  uint8_t status = TWSR & 0xF8;
  return status == 0x18 || status == 0x28;
}

/**
 *  Transmit data from RAM
 *
 *  Transmit \p len bytes via the TWI/I2C interface.
 *  The routine blocks until all data is transmitted.
 *
 *  \param data                  Pointer to data.
 *
 *  \param len                   Number of bytes to transmit.
 */

void ARTX_twi_tx_data(const void *data, uint8_t len)
{
  const uint8_t *p = (const uint8_t *) data;

  while (len--)
  {
    ARTX_twi_tx_byte(*p++);
  }
}

/**
 *  Transmit C string from RAM
 *
 *  Transmit null-terminated string via the TWI/I2C interface.
 *  The routine blocks until the whole string is transmitted.
 *
 *  \param data                  Pointer to null-terminated string.
 */

void ARTX_twi_tx_string(const char *data)
{
  while (*data)
  {
    ARTX_twi_tx_byte(*data++);
  }
}

/**
 *  Transmit C string from flash memory
 *
 *  Transmit null-terminated string from flash memory via the TWI/I2C
 *  interface. The routine blocks until the whole string is transmitted.
 *
 *  \param data                  Pointer to null-terminated string in
 *                               flash memory.
 */

void ARTX_twi_tx_string_P(PGM_P data)
{
  char byte;

  while ((byte = pgm_read_byte(data++)) != 0)
  {
    ARTX_twi_tx_byte(byte);
  }
}



#endif
