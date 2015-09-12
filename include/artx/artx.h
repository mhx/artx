#ifndef artx_ARTX_H_
#define artx_ARTX_H_

/*******************************************************************************
*
* ARTX feature configuration
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
 *  \file artx/artx.h
 *  \brief Features configuration
 */

#define ARTX_STRINGIZE2(str)  #str
#define ARTX_STRINGIZE(str)   ARTX_STRINGIZE2(str)

#ifdef ARTX_CONFIG_H
# include ARTX_STRINGIZE(ARTX_CONFIG_H)
#endif

/**
 *  Enable timing interface
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value enables the timing interface
 *  defined in artx/task.h.
 */
#ifndef ARTX_ENABLE_TIME
# define ARTX_ENABLE_TIME         1
#endif

/**
 *  Enable serial interface
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value enables the serial interface
 *  defined in artx/serial.h.
 */
#ifndef ARTX_ENABLE_SERIAL
# define ARTX_ENABLE_SERIAL       1
#endif

/**
 *  Enable SPI interface
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value enables the SPI interface
 *  defined in artx/spi.h.
 */
#ifndef ARTX_ENABLE_SPI
# define ARTX_ENABLE_SPI          1
#endif

/**
 *  Enable TWI/I2C interface
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value enables the TWI/I2C interface
 *  defined in artx/twi.h.
 */
#ifndef ARTX_ENABLE_TWI
# define ARTX_ENABLE_TWI          1
#endif

/**
 *  Enable EEPROM support
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value enables the EEPROM support
 *  defined in artx/eeprom.h.
 */
#ifndef ARTX_ENABLE_EEPROM
# define ARTX_ENABLE_EEPROM       1
#endif

/**
 *  Enable monitoring support
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value enables the built-in monitoring
 *  support.
 *
 *  It is possible to use this without #ARTX_ENABLE_SERIAL, however,
 *  it doesn't make much sense at the moment as the serial port is
 *  the only way to retrieve the monitoring information. This might
 *  change in future versions.
 *
 *  For obvious reasons, this feature consumes lots of flash, RAM
 *  and run-time. Each task and each routine needs to hold and
 *  update a couple of 32-bit values, which is not an easy job for
 *  an 8-bit microcontroller. But since this is rather a development
 *  feature than something you would use in production code, flash
 *  and RAM usage should not be an issue.
 *
 *  You can use the ARTX monitor (tools/ARTXmon) to retrieve the
 *  monitoring information via the serial port.
 */
#ifndef ARTX_ENABLE_MONITOR
# define ARTX_ENABLE_MONITOR      1
#endif

/**
 *  Routines hold state information
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value makes all routines keep track of
 *  their current state. Right now, the only states a routine can be
 *  in is either #ARTX_RS_DISABLED or #ARTX_RS_ENABLED. When the
 *  routine state is not used (by setting this define to zero), all
 *  routines behave as if they were enabled.
 *
 *  Using the routine state also requires #ARTX_USE_MULTI_ROUT to
 *  be set.
 */
#ifndef ARTX_USE_ROUT_STATE
# define ARTX_USE_ROUT_STATE      1
#endif

/**
 *  Each task can have multiple routines
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value allows you to push multiple routines
 *  on each task's routine queue. This will increase the code size by
 *  about 50 bytes and each routine will use 4 extra bytes of RAM, but
 *  there's not much overhead in terms of run-time.
 */
#ifndef ARTX_USE_MULTI_ROUT
# define ARTX_USE_MULTI_ROUT      1
#endif

/**
 *  Lock calls can be nested
 *
 *  \hideinitializer
 *
 *  Setting this to a nonzero value allows you to nest multiple ARTX_lock()
 *  calls. It is ensured that only the final ARTX_unlock() will actually
 *  release the lock.
 *
 *  This will make each lock/unlock much more expensive in terms of both
 *  flash size and run-time usage, so don't use it unless you really need
 *  it.
 */
#ifndef ARTX_ALLOW_NESTED_LOCKS
# define ARTX_ALLOW_NESTED_LOCKS  0
#endif

/**
 *  Enable synchronization with external time source
 *
 *  \hideinitializer
 *
 *  TODO: more docs
 */
#ifndef ARTX_ENABLE_TICK_SYNC
# define ARTX_ENABLE_TICK_SYNC    1
#endif

#if ARTX_ENABLE_TICK_SYNC

/**
 *  \def ARTX_SYNC_TICKS
 *
 *  Number of kernel ticks per synchronization interval
 *
 *  Set this to the number of kernel ticks that fit exactly
 *  into the interval between two synchronization interrupts.
 *
 *  This must be a multiple of two.
 */
# ifndef ARTX_SYNC_TICKS
#  error "ARTX_SYNC_TICKS is undefined"
# endif

/**
 *  Maximum tick interval adjustment
 *
 *  \hideinitializer
 *
 *  This defines the maximum number of timer increments by
 *  which the kernel tick interval can be modified to
 *  synchronize with the external source. The default is
 *  one percent of the tick interval.
 */
# ifndef ARTX_MAX_SYNC_ADJUST
#  define ARTX_MAX_SYNC_ADJUST (ARTX_TICK_DURATION/100)
# endif

#endif

/**
 *  Kernel tick source
 *
 *  \hideinitializer
 *
 *  This define controls the kernel tick source. The tick source
 *  defines the smallest possible scheduling interval. Whenever
 *  a tick is delivered, the kernel will potentially perform a
 *  task switch if a task with higher priority is ready to be
 *  scheduled.
 *
 *  See artx/tick.h for a list of possible tick sources and
 *  their configuration.
 */
#ifndef ARTX_TICK_SOURCE
# define ARTX_TICK_SOURCE         ARTX_TIMER1_COMPARE
#endif

#if ARTX_USE_ROUT_STATE && !ARTX_USE_MULTI_ROUT
# error "ARTX_USE_ROUT_STATE requires ARTX_USE_MULTI_ROUT"
#endif

/**
 *  System clock frequency
 *
 *  \hideinitializer
 *
 *  Set this to the system clock frequency in hertz. If not set,
 *  either \c F_CPU or \c F_OSC will be used when defined.
 */
#ifndef ARTX_CLOCK_FREQUENCY
# if defined(F_CPU)
#  define ARTX_CLOCK_FREQUENCY F_CPU
# elif defined(F_OSC)
#  define ARTX_CLOCK_FREQUENCY F_OSC
# else
#  error "ARTX_CLOCK_FREQUENCY not set"
# endif
#endif

#endif
