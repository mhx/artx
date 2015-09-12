/*******************************************************************************
*
* ARTX test code
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

#ifndef TESTCONFIG_H_
#define TESTCONFIG_H_

#define ARTX_CLOCK_FREQUENCY   1000000
#define ARTX_SYNC_TICKS            500
#define ARTX_TICK_SOURCE       ARTX_TIMER1_COMPARE

#if defined(__AVR_ATmega16__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega168__)
# define ARTX_TICK_DURATION        2000
#elif defined(__AVR_ATtiny85__)
# define ARTX_TICK_PRESCALER          8
# define ARTX_TICK_DURATION         250
#else
# error "TODO: currently unsupported"
#endif

#define ARTX_ENABLE_TIME 0
#define ARTX_ENABLE_SPI 0
#define ARTX_ENABLE_TWI 0
#define ARTX_ENABLE_SERIAL 0
#define ARTX_ENABLE_MONITOR 0
#define ARTX_ENABLE_TICK_SYNC 0
#define ARTX_USE_ROUT_STATE 0
#define ARTX_USE_MULTI_ROUT 0
#define ARTX_ALLOW_NESTED_LOCKS 0

#endif
