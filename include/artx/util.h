#ifndef artx_UTIL_H_
#define artx_UTIL_H_

/*******************************************************************************
*
* ARTX utility functions
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
 *  \file artx/util.h
 *  \brief Utility functions
 */

#include <avr/io.h>

#include "artx/artx.h"
#include "artx/handy.h"


/* TODO: Locking should be done by disabling the timer interrupt only.
 *       Agreed. Interrupts are safe to run on usertask stacks, as
 *       they can use the context stack, which should be enough.
 *       During interrupts, the global interrupt flag is cleared, so
 *       there are no nesting interrupts. Only drawback is that the
 *       interrupts will potentially disturb monitoring of the usertask
 *       stacks.
 */

#if ARTX_ALLOW_NESTED_LOCKS
/**
 *  Lock Level
 *
 *  \internal
 *
 *  This variable is neccessary to allow nested locks and counts the
 *  number of nesting locks.
 */
extern volatile uint8_t artx_lock_level;
#endif

/**
 *  Lock a user task
 *
 *  Call this routine if you're accessing resources shared between
 *  multiple tasks and you don't want to be interrupted by the
 *  scheduler. When #ARTX_ALLOW_NESTED_LOCKS is defined to 1, locks
 *  may be nested, and interrupts are not enabled before the lock
 *  level drops back to zero. Make sure you lock only for short
 *  periods.
 *
 *  Only the scheduler interrupt is guaranteed to be disabled by
 *  a call to this routine. All other interrupts may still be
 *  delivered.
 *
 *  \see ARTX_unlock(), ARTX_disable_int()
 */
static inline void ARTX_lock(void)
{
  asm volatile ("cli");
#if ARTX_ALLOW_NESTED_LOCKS
  artx_lock_level++;
#endif
}

/**
 *  Unlock a user task
 *
 *  Call this routine after a previous call to ARTX_lock() to
 *  enable the scheduler interrupt again.
 *
 *  \see ARTX_lock()
 */
static inline void ARTX_unlock(void)
{
#if ARTX_ALLOW_NESTED_LOCKS
  /*
   *  XXX: If the lock level is zero here, that's an error.
   */
  if (artx_lock_level-- == 1)
#endif
  {
    asm volatile ("sei");
  }
}

/**
 *  Disable all interrupts
 *
 *  Call this routine if you want to disable all interrupts.
 *  Calls to ARTX_disable_int() and ARTX_enable_int() may never
 *  be nested.
 *
 *  \see ARTX_enable_int(), ARTX_lock()
 */
static inline void ARTX_disable_int(void)
{
  asm volatile ("cli");
}

/**
 *  Enable all interrupts
 *
 *  Call this routine if you want to enable all interrupts after
 *  a previous call to ARTX_disable_int().
 *
 *  \see ARTX_disable_int(), ARTX_lock()
 */
static inline void ARTX_enable_int(void)
{
  asm volatile ("sei");
}

#endif
