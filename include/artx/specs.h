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

#endif
