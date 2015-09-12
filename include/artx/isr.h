#ifndef artx_ISR_H_
#define artx_ISR_H_

/*******************************************************************************
*
* ARTX interrupt service routine support
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
 *  \file artx/isr.h
 *  \brief Interrupt service routine support
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "artx/artx.h"
#include "artx/handy.h"

#if 0 && ARTX_ENABLE_MONITOR

extern uint8_t artx_REG;
extern uint8_t artx_SPL;
extern uint8_t artx_SPH;

void artx_call_isr(void (*isr)(void)) artxNAKED;

# define ARTX_ISR(the_isr)                                              \
                                                                        \
static void artx_isr_ ## the_isr(void);                                 \
                                                                        \
ISR(the_isr) artxNAKED;                                                 \
                                                                        \
ISR(the_isr)                                                            \
{                                                                       \
  asm volatile (                                                        \
                                                                        \
    "sts   artx_REG, r31             \n\t" /* save R31               */ \
                                                                        \
    "in    r31, __SP_L__             \n\t" /* save low byte of SP    */ \
    "sts   artx_SPL, r31             \n\t"                              \
    "in    r31, __SP_H__             \n\t" /* save high byte of SP   */ \
    "sts   artx_SPH, r31             \n\t"                              \
                                                                        \
    "ldi   r31, lo8(__stack)         \n\t" /* switch to kernel stack */ \
    "out   __SP_L__, r31             \n\t"                              \
    "ldi   r31, hi8(__stack)         \n\t"                              \
    "out   __SP_H__, r31             \n\t"                              \
                                                                        \
  );                                                                    \
                                                                        \
  artx_call_isr(&artx_isr_ ## the_isr);                                 \
}                                                                       \
                                                                        \
void artx_isr_ ## the_isr(void)

#else // !ARTX_ENABLE_MONITOR

# define ARTX_ISR(the_isr)   ISR(the_isr)

#endif // ARTX_ENABLE_MONITOR

#endif
