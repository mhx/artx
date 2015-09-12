#ifndef artx_TICK_H_
#define artx_TICK_H_

/*******************************************************************************
*
* ARTX tick source configuration
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
 *  \file artx/tick.h
 *  \brief Tick source configuration
 */

#include "artx/artx.h"


/**
 *  Use Timer/Counter0 overflow as tick source
 *
 *  \hideinitializer
 *
 *  Setting #ARTX_TICK_SOURCE to this value will trigger the
 *  kernel tick whenever the Timer/Counter0 overflows. This
 *  can be further controlled by setting #ARTX_TICK_PRESCALER.
 */
#define ARTX_TIMER0_OVERFLOW 1

/**
 *  Use Timer/Counter1 overflow as tick source
 *
 *  \hideinitializer
 *
 *  Setting #ARTX_TICK_SOURCE to this value will trigger the
 *  kernel tick whenever the Timer/Counter1 overflows. This
 *  can be further controlled by setting #ARTX_TICK_PRESCALER.
 *
 *  \note It doesn't make much sense to waste the Timer/Counter1
 *        for this, so this setting is currently unsupported.
 */
#define ARTX_TIMER1_OVERFLOW 2

/**
 *  Use Timer/Counter1 compare as tick source
 *
 *  \hideinitializer
 *
 *  Setting #ARTX_TICK_SOURCE to this value will trigger the
 *  kernel tick whenever the Timer/Counter1 reaches the value
 *  set by #ARTX_TICK_DURATION. This can be further controlled
 *  by setting #ARTX_TICK_PRESCALER.
 *
 *  This is the most precise way to configure your kernel tick.
 *  If you don't use the prescaler, you can configure the exact
 *  number of clock cycles per tick using #ARTX_TICK_DURATION.
 */
#define ARTX_TIMER1_COMPARE 3

/**
 *  Use external interrupt as tick source
 *
 *  \hideinitializer
 *
 *  Setting #ARTX_TICK_SOURCE to this value will trigger the
 *  kernel tick on an external interrupt.
 */
#define ARTX_TICK_EXTERNAL_INT 4

/**
 *  Choose prescaler for tick source
 *
 *  \hideinitializer
 *
 *  Set this to one of the allowed prescaler values for the
 *  chosen tick source.
 */
#ifndef ARTX_TICK_PRESCALER
# define ARTX_TICK_PRESCALER 1
#endif

/**
 *  Set tick duration
 *
 *  \hideinitializer
 *
 *  This can be used to set the exact duration of a kernel tick
 *  when #ARTX_TIMER1_COMPARE has been chosen as the tick source.
 */
#ifndef ARTX_TICK_DURATION
# define ARTX_TICK_DURATION (~1)
#endif

/**
 *  \def artx_PRESCALER
 *
 *  Prescaler bit pattern
 *
 *  \internal
 *  \hideinitializer
 *
 *  This is the bit pattern that needs to be written to set the
 *  prescaler.
 */

#if ARTX_TICK_SOURCE == ARTX_TIMER0_OVERFLOW

# if ARTX_TICK_PRESCALER == 1
#  define artx_PRESCALER             (1 << CS00)
# elif ARTX_TICK_PRESCALER == 8
#  define artx_PRESCALER             (1 << CS01)
# elif ARTX_TICK_PRESCALER == 64
#  define artx_PRESCALER            ((1 << CS01) | (1 << CS00))
# elif ARTX_TICK_PRESCALER == 256
#  define artx_PRESCALER             (1 << CS02)
# elif ARTX_TICK_PRESCALER == 1024
#  define artx_PRESCALER            ((1 << CS02) | (1 << CS00))
# else
#  error "Invalid prescaler for chosen tick source"
# endif

#elif ARTX_TICK_SOURCE == ARTX_TIMER1_OVERFLOW || \
      ARTX_TICK_SOURCE == ARTX_TIMER1_COMPARE

# if defined(__AVR_ATmega16__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega168__)

#  if ARTX_TICK_PRESCALER == 1
#   define artx_PRESCALER             (1 << CS10)
#  elif ARTX_TICK_PRESCALER == 8
#   define artx_PRESCALER             (1 << CS11)
#  elif ARTX_TICK_PRESCALER == 64
#   define artx_PRESCALER            ((1 << CS11) | (1 << CS10))
#  elif ARTX_TICK_PRESCALER == 256
#   define artx_PRESCALER             (1 << CS12)
#  elif ARTX_TICK_PRESCALER == 1024
#   define artx_PRESCALER            ((1 << CS12) | (1 << CS10))
#  else
#   error "Invalid prescaler for chosen tick source"
#  endif

# elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)

#  if ARTX_TICK_PRESCALER == 1
#   define artx_PRESCALER                                                       (1 << CS10)
#  elif ARTX_TICK_PRESCALER == 2
#   define artx_PRESCALER                                         (1 << CS11)
#  elif ARTX_TICK_PRESCALER == 4
#   define artx_PRESCALER                                        ((1 << CS11) | (1 << CS10))
#  elif ARTX_TICK_PRESCALER == 8
#   define artx_PRESCALER                           (1 << CS12)
#  elif ARTX_TICK_PRESCALER == 16
#   define artx_PRESCALER                          ((1 << CS12) |               (1 << CS10))
#  elif ARTX_TICK_PRESCALER == 32
#   define artx_PRESCALER                          ((1 << CS12) | (1 << CS11))
#  elif ARTX_TICK_PRESCALER == 64
#   define artx_PRESCALER                          ((1 << CS12) | (1 << CS11) | (1 << CS10))
#  elif ARTX_TICK_PRESCALER == 128
#   define artx_PRESCALER             (1 << CS13)
#  elif ARTX_TICK_PRESCALER == 256
#   define artx_PRESCALER            ((1 << CS13) |                             (1 << CS10))
#  elif ARTX_TICK_PRESCALER == 512
#   define artx_PRESCALER            ((1 << CS13) |               (1 << CS11))
#  elif ARTX_TICK_PRESCALER == 1024
#   define artx_PRESCALER            ((1 << CS13) |              ((1 << CS11) | (1 << CS10))
#  elif ARTX_TICK_PRESCALER == 2048
#   define artx_PRESCALER            ((1 << CS13) | (1 << CS12)
#  elif ARTX_TICK_PRESCALER == 4096
#   define artx_PRESCALER            ((1 << CS13) | (1 << CS12) |               (1 << CS10))
#  elif ARTX_TICK_PRESCALER == 8192
#   define artx_PRESCALER            ((1 << CS13) | (1 << CS12) | (1 << CS11))
#  elif ARTX_TICK_PRESCALER == 16384
#   define artx_PRESCALER            ((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10))
#  else
#   error "Invalid prescaler for chosen tick source"
#  endif

# else
#  error "TODO: currently unsupported"
# endif

#endif

/**
 *  \def ARTX_TICK_INIT
 *
 *  Initialize tick source
 *
 *  \hideinitializer
 *
 *  Call this before ARTX_schedule() to initialize the kernel tick
 *  source.
 */

/**
 *  \def artx_TIMER_REG
 *
 *  Timer register
 *
 *  \internal
 *  \hideinitializer
 *
 *  The timer register used for the selected tick source.
 */

/**
 *  \def artx_TIMER_TYPE
 *
 *  Timer register type
 *
 *  \internal
 *  \hideinitializer
 *
 *  A type large enough to hold all values of #artx_TIMER_REG.
 */

/**
 *  \def artx_TIMER_TOP
 *
 *  Maximum timer value
 *
 *  \internal
 *  \hideinitializer
 *
 *  The value of the timer register at which the tick interrupt
 *  will be called.
 */

/**
 *  \def artx_TICK_VECTOR
 *
 *  Tick source interrupt vector
 *
 *  \internal
 *  \hideinitializer
 *
 *  This will be set to the interrupt vector for the selected
 *  tick source.
 */

/**
 *  \def artx_TICK_ADJUST
 *
 *  Adjust tick length for synchronization
 *
 *  \internal
 *  \hideinitializer
 *
 *  \param  amount               The correction in same unit as
 *                               #ARTX_TICK_DURATION.
 */

/**
 *  \def artx_CUR_TIMER_TOP
 *
 *  Get the current corrected timer top value
 *
 *  \internal
 *  \hideinitializer
 */

/**
 *  \def SIMULAVR_TICK_SIGNAL
 *
 *  Tick signal for simulavr
 *
 *  \internal
 *  \hideinitializer
 *
 *  This define is used by the test suite to determine the gdb
 *  signal that needs to be sent to simulavr to trigger the
 *  tick interrupt.
 */

# if defined(TIMSK0)
#  define artx_TIMSK0 TIMSK0
#  define artx_TIMSK1 TIMSK1
# else
#  define artx_TIMSK0 TIMSK
#  define artx_TIMSK1 TIMSK
# endif

//=====================================================================
#if ARTX_TICK_SOURCE == ARTX_TIMER0_OVERFLOW
//=====================================================================

# if defined TIMER0_OVF_vect
#  define artx_TICK_VECTOR        TIMER0_OVF_vect
# elif defined TIM0_OVF_vect
#  define artx_TICK_VECTOR        TIM0_OVF_vect
# else
#  error No suitable tick vector found
# endif

# define artx_TIMER_REG           TCNT0
# define artx_TIMER_TYPE          uint8_t

# define artx_TIMER_TOP           256

# define ARTX_TICK_INIT                                               \
         do {                                                         \
           TCCR0 = artx_PRESCALER;                                    \
           artx_TIMSK0 |= (1 << TOIE0);                               \
         } while (0)

# if ARTX_ENABLE_TICK_SYNC
#  error "ARTX_ENABLE_TICK_SYNC is not supported for this tick source"
# endif

//=====================================================================
#elif ARTX_TICK_SOURCE == ARTX_TIMER1_OVERFLOW
//=====================================================================

# error "TODO: currently unsupported"

//=====================================================================
#elif ARTX_TICK_SOURCE == ARTX_TIMER1_COMPARE
//=====================================================================

# if defined(__AVR_ATmega16__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega168__)

#  define artx_TICK_VECTOR         TIMER1_COMPA_vect

#  define artx_TIMER_REG           TCNT1
#  define artx_TIMER_TYPE          uint16_t

#  define artx_TIMER_TOP           (ARTX_TICK_DURATION - 1)

#  define ARTX_TICK_INIT                                              \
          do {                                                        \
            TCCR1B = (1 << WGM12) | artx_PRESCALER;                   \
            OCR1A = artx_TIMER_TOP;                                   \
            artx_TIMSK1 |= (1 << OCIE1A);                             \
          } while (0)

#  if ARTX_ENABLE_TICK_SYNC

#   define artx_CUR_TIMER_TOP       OCR1A

#   define artx_TICK_ADJUST(amount)                                   \
           do {                                                       \
             OCR1A = artx_TIMER_TOP + (amount);                       \
           } while (0)

#  endif

# elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)

#  define artx_TICK_VECTOR         TIMER1_COMPA_vect

#  define artx_TIMER_REG           TCNT1
#  define artx_TIMER_TYPE          uint8_t

#  define artx_TIMER_TOP           (ARTX_TICK_DURATION - 1)

#  define ARTX_TICK_INIT                                              \
          do {                                                        \
            TCCR1 = (1 << CTC1) | artx_PRESCALER;                     \
            OCR1A = artx_TIMER_TOP;                                   \
            OCR1C = artx_TIMER_TOP;                                   \
            artx_TIMSK1 |= (1 << OCIE1A);                             \
          } while (0)

#  if ARTX_ENABLE_TICK_SYNC

#   define artx_CUR_TIMER_TOP       OCR1A

#   define artx_TICK_ADJUST(amount)                                   \
           do {                                                       \
             OCR1A = artx_TIMER_TOP + (amount);                       \
             OCR1C = artx_TIMER_TOP + (amount);                       \
           } while (0)

#  endif

# else
#  error "TODO: currently unsupported"
# endif

//=====================================================================
#elif ARTX_TICK_SOURCE == ARTX_TICK_EXTERNAL_INT
//=====================================================================

# define artx_TICK_VECTOR         ARTX_TICK_INT_VECTOR

# define artx_TIMER_REG           0
# define artx_TIMER_TYPE          uint16_t

# define artx_TIMER_TOP           0

# define ARTX_TICK_INIT                                               \
         do {} while (0)

//=====================================================================
#else
//=====================================================================

# error "Unsupported ARTX_TICK_SOURCE"

#endif

#endif
