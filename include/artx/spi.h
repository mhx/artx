#ifndef artx_SPI_H_
#define artx_SPI_H_

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
 *  \file artx/spi.h
 *  \brief SPI interface
 */

#include <avr/pgmspace.h>

#include "artx/artx.h"

#if ARTX_ENABLE_SPI

/**
 *  SPI data order
 */
enum ARTX_spi_order
{
  ARTX_SPI_ORDER_MSB_FIRST,      //!< MSB is transmitted first
  ARTX_SPI_ORDER_LSB_FIRST       //!< LSB is transmitted first
};

/**
 *  SPI clock polarity
 */
enum ARTX_spi_polarity
{
  ARTX_SPI_POLARITY_LOW,         //!< SPI clock is low when idle
  ARTX_SPI_POLARITY_HIGH         //!< SPI clock is high when idle
};

/**
 *  SPI clock phase
 */
enum ARTX_spi_phase
{
  ARTX_SPI_PHASE_LEADING,        //!< Data sampled on SPI clock leading edge
  ARTX_SPI_PHASE_TRAILING        //!< Data sampled on SPI clock trailing edge
};

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

static inline void ARTX_spi_master_init(uint32_t max_rate,
                                        enum ARTX_spi_order order,
                                        enum ARTX_spi_polarity polarity,
                                        enum ARTX_spi_phase phase,
                                        uint8_t double_speed_mode)
{
  uint8_t scale = (ARTX_CLOCK_FREQUENCY/(double_speed_mode ? 2 : 4) + max_rate/2)/max_rate;
  uint8_t spr = scale > 1 ? scale > 4 ? scale > 16 ? 3 : 2 : 1 : 0;

  SPSR = (double_speed_mode ? 1 : 0) << SPI2X;

  SPCR = (1 << MSTR)        |
         (1 << SPE)         |
         (order << DORD)    |
         (polarity << CPOL) |
         (phase << CPHA)    |
         (spr << SPR0);
}

void ARTX_spi_tx_byte(uint8_t data);
uint8_t ARTX_spi_rx_byte(void);
void ARTX_spi_tx_data(const void *data, uint8_t len);
void ARTX_spi_tx_string(const char *data);
void ARTX_spi_tx_string_P(PGM_P data);

#endif

#endif
