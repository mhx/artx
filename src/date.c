/*******************************************************************************
*
* ARTX date functions
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
 *  \file date.c
 *  \brief Date functions
 */


/*===== GLOBAL INCLUDES ======================================================*/

#include <avr/pgmspace.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/date.h"


/*===== DEFINES ==============================================================*/

#ifndef ARTX_DAY_OF_WEEK_HIGH_PREC
# define ARTX_DAY_OF_WEEK_HIGH_PREC 0
#endif

/*===== TYPEDEFS =============================================================*/

/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

/*===== EXTERNAL VARIABLES ===================================================*/

/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

/*===== STATIC FUNCTIONS =====================================================*/

/*===== FUNCTIONS ============================================================*/

/**
 *  Compute day-of-week from date
 *
 *  A version of Sakamoto's algorithm, optimized to require only basic 8-bit
 *  operations (add, sub, compare and shift).
 *
 *  Accurate from 0001-01-01 until 2954-10-31. When ARTX_DAY_OF_WEEK_HIGH_PREC
 *  is 1, accurate until 5754-10-31.
 *
 *  \param c                     Century (0 - 57).
 *  \param y                     Year (0 - 99).
 *  \param m                     Month (1 - 12).
 *  \param d                     Day (1 - 31).
 *
 *  \return Day of week. 0 = Sunday, 1 = Monday, ..., 6 = Saturday
 */
uint8_t ARTX_day_of_week(uint8_t c, uint8_t y, uint8_t m, uint8_t d)
{
  static const uint8_t mm[12] PROGMEM = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  if (m < 3)
  {
    if (y == 0)
    {
      y = 100;
      c--;
    }
    y--;
  }
  if (y >= 56) y -= 56;
#if ARTX_DAY_OF_WEEK_HIGH_PREC
  if (c >= 28) c -= 28;
#endif
  d += (c << 2) + c + (c >> 2) + y + (y >> 2) + pgm_read_byte(mm + m - 1);
  d = (d & 7) + ((d >> 3) & 7) + (d >> 6);
  d = (d >> 3) + (d & 7);
  return d < 7 ? d : d - 7;
}
