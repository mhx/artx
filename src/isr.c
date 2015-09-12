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
 *  \file isr.c
 *  \brief Interrupt service routine support
 */

/*===== GLOBAL INCLUDES ======================================================*/

/*===== LOCAL INCLUDES =======================================================*/

#include "artx/isr.h"


/*===== DEFINES ==============================================================*/

/*===== TYPEDEFS =============================================================*/

/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

/*===== EXTERNAL VARIABLES ===================================================*/

/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

#if 0 && ARTX_ENABLE_MONITOR

uint8_t artxASMONLY artx_REG;  //!< register temporary storage \internal
uint8_t artxASMONLY artx_SPL;  //!< stack ptr lo-byte temp storage \internal
uint8_t artxASMONLY artx_SPH;  //!< stack ptr hi-byte temp storage \internal

#endif // ARTX_ENABLE_MONITOR


/*===== STATIC FUNCTIONS =====================================================*/

/*===== FUNCTIONS ============================================================*/

#if 0 && ARTX_ENABLE_MONITOR

void artx_call_isr(void (*isr)(void))
{
  asm volatile (

    "in    r31, __SREG__             \n\t" /* load SREG              */

    "push  r31                       \n\t" /* save registers         */
    "push  r30                       \n\t"
    "push  r29                       \n\t"
    "push  r28                       \n\t"
    "push  r27                       \n\t"
    "push  r26                       \n\t"
    "push  r25                       \n\t"
    "push  r24                       \n\t"
    "push  r23                       \n\t"
    "push  r22                       \n\t"
    "push  r21                       \n\t"
    "push  r20                       \n\t"
    "push  r19                       \n\t"
    "push  r18                       \n\t"
    "push  r17                       \n\t"
    "push  r16                       \n\t"
    "push  r15                       \n\t"
    "push  r14                       \n\t"
    "push  r13                       \n\t"
    "push  r12                       \n\t"
    "push  r11                       \n\t"
    "push  r10                       \n\t"
    "push  r9                        \n\t"
    "push  r8                        \n\t"
    "push  r7                        \n\t"
    "push  r6                        \n\t"
    "push  r5                        \n\t"
    "push  r4                        \n\t"
    "push  r3                        \n\t"
    "push  r2                        \n\t"
    "push  r1                        \n\t"
    "push  r0                        \n\t"

    "clr   __zero_reg__              \n\t"

  );

  isr();

  asm volatile (

    "pop   r0                        \n\t" /* restore registers      */
    "pop   r1                        \n\t"
    "pop   r2                        \n\t"
    "pop   r3                        \n\t"
    "pop   r4                        \n\t"
    "pop   r5                        \n\t"
    "pop   r6                        \n\t"
    "pop   r7                        \n\t"
    "pop   r8                        \n\t"
    "pop   r9                        \n\t"
    "pop   r10                       \n\t"
    "pop   r11                       \n\t"
    "pop   r12                       \n\t"
    "pop   r13                       \n\t"
    "pop   r14                       \n\t"
    "pop   r15                       \n\t"
    "pop   r16                       \n\t"
    "pop   r17                       \n\t"
    "pop   r18                       \n\t"
    "pop   r19                       \n\t"
    "pop   r20                       \n\t"
    "pop   r21                       \n\t"
    "pop   r22                       \n\t"
    "pop   r23                       \n\t"
    "pop   r24                       \n\t"
    "pop   r25                       \n\t"
    "pop   r26                       \n\t"
    "pop   r27                       \n\t"
    "pop   r28                       \n\t"
    "pop   r29                       \n\t"
    "pop   r30                       \n\t"
    "pop   r31                       \n\t"

    "out   __SREG__, r31             \n\t" /* restore SREG           */

    "lds   r31, artx_SPL             \n\t" /* restore SP             */
    "out   __SP_L__, r31             \n\t"
    "lds   r31, artx_SPH             \n\t"
    "out   __SP_H__, r31             \n\t"

    "lds   r31, artx_REG             \n\t" /* restore R31            */

    "reti                            \n\t" /* return from interrupt  */

  );
}

#endif // ARTX_ENABLE_MONITOR

