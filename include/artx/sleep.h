#ifndef artx_SLEEP_H_
#define artx_SLEEP_H_

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
 *  \file artx/sleep.h
 *  \brief Sleeping routines
 */

#include <stdint.h>

#include "artx/artx.h"
#include "artx/handy.h"

void ARTX_sleep(uint16_t seconds);

void ARTX_millisleep(uint16_t milliseconds);

static inline void artx_sleep_four_cycles(uint16_t loops)
{
  asm volatile(
    "1: sbiw %0, 1 \n\t"
    "   brne 1b    \n\t"
    : "=w" (loops)
    : "0" (loops)
  );
}

#define ARTX_microsleep(microseconds)                                                                  \
        do {                                                                                           \
          ARTX_STATIC_ASSERT(microseconds <= ((uint64_t)UINT16_MAX*4000000UL)/ARTX_CLOCK_FREQUENCY);   \
          artx_sleep_four_cycles(((uint64_t) ARTX_CLOCK_FREQUENCY*microseconds)/4000000UL);            \
        } while (0)

#define ARTX_nanosleep(nanoseconds)                                                                    \
        do {                                                                                           \
          ARTX_STATIC_ASSERT(nanoseconds <= ((uint64_t)UINT16_MAX*4000000000UL)/ARTX_CLOCK_FREQUENCY); \
          artx_sleep_four_cycles(((uint64_t) ARTX_CLOCK_FREQUENCY*nanoseconds)/4000000000UL);          \
        } while (0)

#endif
