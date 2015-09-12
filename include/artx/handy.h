#ifndef artx_HANDY_H_
#define artx_HANDY_H_

/*******************************************************************************
*
* ARTX handy stuff
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
 *  \file artx/handy.h
 *  \brief Handy stuff
 */

/**
 *  Declare as used by assembly only
 *
 *  \internal
 */
#define artxASMONLY        __attribute__((used))

/**
 *  Declare as unused
 *
 *  \internal
 */
#define artxUNUSED         __attribute__((unused))

/**
 *  Declare function that does not return
 *
 *  \internal
 */
#define artxNORETURN       __attribute__((noreturn))

/**
 *  Declare function that must not be inlined
 *
 *  \internal
 */
#define artxNEVERINLINE    __attribute__((noinline))

/**
 *  Declare function that must be inlined
 *
 *  \internal
 */
#define artxALWAYSINLINE   __attribute__((always_inline))

/**
 *  Declare function as naked
 *
 *  \internal
 */
#define artxNAKED          __attribute__((naked))

/**
 *  Static assertion
 *
 *  Use this macro for static (compile time) assertions,
 *  just like you would use the standard assert macro.
 *
 *  The difference is that you can check things at compile
 *  time and be sure to get a compile time error if the
 *  assertion fails.
 */
#define ARTX_STATIC_ASSERT(cond)                                           \
            extern artxUNUSED int _artx_STATIC_ASSERTION_FAILED_[(cond) ? 1 : -1]

/**
 *  Likely to be true condition
 *
 *  \internal
 *  \hideinitializer
 */
#define artxLIKELY(cond)     __builtin_expect(!!(cond), 1)

/**
 *  Unlikely to be true condition
 *
 *  \internal
 *  \hideinitializer
 */
#define artxUNLIKELY(cond)   __builtin_expect(!!(cond), 0)

#endif
