/*******************************************************************************
*
* ARTX SPI interface
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
 *  \brief SPI interface
 */

// TODO: see if we can do buffering and use the interrupt to transmit data


/*===== GLOBAL INCLUDES ======================================================*/

#include <avr/io.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/spi.h"

#if ARTX_ENABLE_SPI

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

void ARTX_spi_tx_byte(uint8_t data)
{
  SPDR = data;

#ifndef ARTX_UNDER_TEST
  while ((SPSR & (1 << SPIF)) == 0)
  {
    (void) 0;
  }
#endif
}

/**
 *  Receive a single byte
 *
 *  Transmit a single byte via the SPI interface.
 *  The routine blocks until the byte is transmitted.
 *
 *  \return Received byte
 */

uint8_t ARTX_spi_rx_byte(void)
{
  return SPDR;
}

/**
 *  Transmit data from RAM
 *
 *  Transmit \p len bytes via the SPI interface.
 *  The routine blocks until all data is transmitted.
 *
 *  \param data                  Pointer to data.
 *
 *  \param len                   Number of bytes to transmit.
 */

void ARTX_spi_tx_data(const void *data, uint8_t len)
{
  const uint8_t *p = (const uint8_t *) data;

  while (len--)
  {
    ARTX_spi_tx_byte(*p++);
  }
}

/**
 *  Transmit C string from RAM
 *
 *  Transmit null-terminated string via the SPI interface.
 *  The routine blocks until the whole string is transmitted.
 *
 *  \param data                  Pointer to null-terminated string.
 */

void ARTX_spi_tx_string(const char *data)
{
  while (*data)
  {
    ARTX_spi_tx_byte(*data++);
  }
}

/**
 *  Transmit C string from flash memory
 *
 *  Transmit null-terminated string from flash memory via the SPI
 *  interface. The routine blocks until the whole string is transmitted.
 *
 *  \param data                  Pointer to null-terminated string in
 *                               flash memory.
 */

void ARTX_spi_tx_string_P(PGM_P data)
{
  char byte;

  while ((byte = pgm_read_byte(data++)) != 0)
  {
    ARTX_spi_tx_byte(byte);
  }
}

#endif
