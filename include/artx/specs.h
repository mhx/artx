#ifndef artx_SPECS_H_
#define artx_SPECS_H_

/*******************************************************************************
*
* ARTX AVR specifications
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
 *  \file artx/specs.h
 *  \brief AVR specifications
 */

#if defined(__AVR_ENHANCED__) && __AVR_ENHANCED__
#  define artx_HAVE_MUL 1
#  define artx_HAVE_MOVW 1
#endif

# if defined(__AVR_ATmega16__) || \
     defined(__AVR_ATmega16U4__) || \
     defined(__AVR_ATmega32__) || \
     defined(__AVR_ATmega32U4__) || \
    /* 164/324/644/1284 series */ \
     defined(__AVR_ATmega164__) || \
     defined(__AVR_ATmega164A__) || \
     defined(__AVR_ATmega164P__) || \
     defined(__AVR_ATmega324__) || \
     defined(__AVR_ATmega324A__) || \
     defined(__AVR_ATmega324P__) || \
     defined(__AVR_ATmega324PA__) || \
     defined(__AVR_ATmega644__) || \
     defined(__AVR_ATmega644A__) || \
     defined(__AVR_ATmega644P__) || \
     defined(__AVR_ATmega644PA__) || \
     defined(__AVR_ATmega1284P__) || \
    /* 48/88/168/328 series */ \
     defined(__AVR_ATmega48__) || \
     defined(__AVR_ATmega48A__) || \
     defined(__AVR_ATmega48P__) || \
     defined(__AVR_ATmega88__) || \
     defined(__AVR_ATmega88A__) || \
     defined(__AVR_ATmega88P__) || \
     defined(__AVR_ATmega88PA__) || \
     defined(__AVR_ATmega168__) || \
     defined(__AVR_ATmega168A__) || \
     defined(__AVR_ATmega168P__) || \
     defined(__AVR_ATmega328__) || \
     defined(__AVR_ATmega328P__)

#  define artx_TIMER1_BITS 16

# elif defined(__AVR_ATtiny25__) || \
       defined(__AVR_ATtiny45__) || \
       defined(__AVR_ATtiny85__)

#  define artx_TIMER1_BITS 8

#endif

#endif
