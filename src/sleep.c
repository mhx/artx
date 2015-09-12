/*******************************************************************************
*
* ARTX sleeping routines
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
 *  \file sleep.c
 *  \brief Sleeping routines
 */

#include "artx/sleep.h"

void ARTX_sleep(uint16_t seconds)
{
  while (seconds--)
  {
    ARTX_millisleep(1000);
  }
}

void ARTX_millisleep(uint16_t milliseconds)
{
  while (milliseconds--)
  {
    ARTX_microsleep(1000);
  }
}

