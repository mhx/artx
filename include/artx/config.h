#ifndef artx_CONFIG_H_
#define artx_CONFIG_H_

/*******************************************************************************
*
* ARTX example configuration file
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

#define ARTX_ENABLE_SERIAL       1
#define ARTX_ENABLE_SPI          1
#define ARTX_ENABLE_TWI          0
#define ARTX_ENABLE_EEPROM       0
#define ARTX_ENABLE_MONITOR      0
#define ARTX_ENABLE_TICK_SYNC    0
#define ARTX_ENABLE_TIME         1

#define ARTX_USE_ROUT_STATE      0
#define ARTX_USE_MULTI_ROUT      0

#define ARTX_ALLOW_NESTED_LOCKS  0

#define ARTX_DEFAULT_SERIAL_UART 1

// #define ARTX_CLOCK_FREQUENCY     14745600
// #define ARTX_CLOCK_FREQUENCY     11059200
// #define ARTX_CLOCK_FREQUENCY     16000000

#define ARTX_TICK_SOURCE         ARTX_TIMER1_COMPARE
// #define ARTX_TICK_DURATION       22118
// #define ARTX_TICK_DURATION       13824
#define ARTX_TICK_DURATION       2500
#define ARTX_TICK_PRESCALER        64

// #define ARTX_SYNC_TICKS          100

// #define ARTX_TICK_DURATION       32000

// #define ARTX_TICK_SOURCE         ARTX_TIMER0_OVERFLOW
// #define ARTX_TICK_PRESCALER      64

#endif
