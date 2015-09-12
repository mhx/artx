#ifndef artx_TWI_H_
#define artx_TWI_H_

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
 *  \file artx/twi.h
 *  \brief TWI/I2C interface
 */

#include <avr/pgmspace.h>

#include "artx/artx.h"

#if ARTX_ENABLE_TWI

/**
 *  Initialize SPI interface
 *
 *  Call this routine to (re-)initialize the SPI interface.
 *  The clock rate will be adjusted to the closest matching
 *  rate less than the specified \p max_rate that is available
 *  for the current #ARTX_CLOCK_FREQUENCY.
 *
 *  \param mode                  SPI mode.
 *
 *  \param max_rate              Maximum SPI clock rate. This parameter
 *                               is only used in master mode.
 */

static inline void ARTX_twi_master_init(uint32_t rate)
{
  uint8_t prescale = ((ARTX_CLOCK_FREQUENCY/rate) - 16)/255;
  uint8_t psbits = prescale < 2 ? 0 : prescale < 8 ? 1 : prescale < 32 ? 2 : 3;
  uint8_t brbits = ((ARTX_CLOCK_FREQUENCY/rate) - 16 + (1 << (2*psbits)))/(2 << (2*psbits));

  TWBR = brbits;
  TWCR |= TWEN;
  TWSR = (psbits << TWPS0);
  PORTD |= (1 << DDD1) | (1 << DDD0);
}

uint8_t ARTX_twi_start(uint8_t addr, uint8_t write);
void ARTX_twi_stop(void);

uint8_t ARTX_twi_tx_byte(uint8_t data);
void ARTX_twi_tx_data(const void *data, uint8_t len);
void ARTX_twi_tx_string(const char *data);
void ARTX_twi_tx_string_P(PGM_P data);

#endif

#endif
