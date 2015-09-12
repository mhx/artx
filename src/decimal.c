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
 *  \file decimal.c
 *  \brief Decimal conversion functions
 */


/*===== GLOBAL INCLUDES ======================================================*/

#include <avr/io.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/decimal.h"


/*===== DEFINES ==============================================================*/

/*===== TYPEDEFS =============================================================*/

/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

/*===== EXTERNAL VARIABLES ===================================================*/

/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

/*===== STATIC FUNCTIONS =====================================================*/

/*===== FUNCTIONS ============================================================*/

/*
   divide by 10 with remainder

             GCC    ARTX   factor
   ------------------------------
   32-bit    1310    157   8.3
   16-bit     429     67   6.4
    8-bit     180     50   3.6

*/

uint32_t ARTX_div10_u32(uint32_t n, uint8_t *remainder)
{
  uint32_t q;
  uint8_t r;

  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  n -= q & (~0x7);
  q = q >> 3;
  r = n - (q << 1);

  if (remainder)
  {
    *remainder = r > 9 ? r - 10 : r;
  }

  return q + (r > 9);
}

uint16_t ARTX_div10_u16(uint16_t n, uint8_t *remainder)
{
  uint16_t q;
  uint8_t r;

  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q >> 3;
  r = n - (q << 1) - (q << 3);

  if (remainder)
  {
    *remainder = r > 9 ? r - 10 : r;
  }

  return q + (r > 9);
}

uint8_t ARTX_div10_u8(uint8_t n, uint8_t *remainder)
{
  uint8_t q;
  uint8_t r;

  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q >> 3;
  r = n - q*10;

  if (remainder)
  {
    *remainder = r > 9 ? r - 10 : r;
  }

  return q + (r > 9);
}

uint8_t ARTX_dec2bcd_u8(uint8_t dec)
{
  uint8_t ten, one;

  ten = ARTX_div10_u8(dec, &one);

  return (ten << 4) | one;
}

uint16_t ARTX_dec2bcd_u16(uint16_t dec)
{
  uint16_t bcd = 0;
  uint8_t rem;

  dec = ARTX_div10_u16(dec, &rem);
  bcd |= rem;
  dec = ARTX_div10_u16(dec, &rem);
  bcd |= rem << 4;
  bcd |= ARTX_dec2bcd_u8(dec) << 8;

  return bcd;
}

uint32_t ARTX_dec2bcd_u32(uint32_t dec)
{
  uint32_t bcd = 0;
  uint8_t rem;

  dec = ARTX_div10_u32(dec, &rem);
  bcd |= rem;
  dec = ARTX_div10_u32(dec, &rem);
  bcd |= rem << 4;
  dec = ARTX_div10_u32(dec, &rem);
  bcd |= rem << 8;
  dec = ARTX_div10_u32(dec, &rem);
  bcd |= rem << 12;
  bcd |= (uint32_t) ARTX_dec2bcd_u16(dec) << 16;

  return bcd;
}

uint8_t ARTX_bcd2dec_u8(uint8_t bcd)
{
  uint8_t hi = bcd >> 4;
  return ((bcd >> 1) & 0x78) + (hi << 1) + (bcd & 0x0F);
}
