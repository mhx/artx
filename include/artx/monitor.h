#ifndef artx_MONITOR_H_
#define artx_MONITOR_H_

/*******************************************************************************
*
* ARTX monitoring support
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
 *  \file artx/monitor.h
 *  \brief Monitoring support
 */

#include <avr/pgmspace.h>

#include "artx/artx.h"

#if ARTX_ENABLE_MONITOR

/**
 *  Extra stack size
 *
 *  \internal
 *
 *  The number of bytes to reserve on top of what the user has
 *  requested. This extra amount is used to identify stack
 *  overruns in monitoring mode.
 */
#define artx_MONITOR_EXTRA_STACK  8

/**
 *  Monitor protocol version
 *
 *  \internal
 *
 *  The version of the monitor protocol. To be used for future
 *  enhancements.
 */
#define artx_MONITOR_VERSION      0

/**
 *  Monitoring message header
 *
 *  \internal
 *
 *  This header is sent directly after the message marker.
 *  It contains information about various constants used by
 *  the ARTX monitor.
 */
struct artx_monitor_header
{
  uint8_t  version;              //!< Protocol version
  uint8_t  hdr_size;             //!< Size of this structure
  uint8_t  tcb_size;             //!< Size of task control block
  uint8_t  rcb_size;             //!< Size of routine control block
  uint16_t nom_tick_duration;    //!< Nominal tick duration in counter units
  uint16_t cur_tick_duration;    //!< Current tick duration in counter units
  uint16_t tick_prescaler;       //!< Counter prescaler used for tick
  uint16_t monitor_interval;     //!< Monitoring interval in ticks
  uint32_t clock_frequency;      //!< System clock frequency
};

/**
 *  Monitoring state
 *
 *  \internal
 *
 *  Indicates the monitoring state of each task and routine.
 *
 *  At the end of the monitoring interval, the state of all tasks
 *  and routines that have been run at least once is changed from
 *  #artx_MS_COLLECT to #artx_MS_READY in artx_yield().
 *  This will prevent further updates and ensure a consistent set
 *  of data when the information is transmitted. After it has been
 *  transmitted by artx_monitor_transmit(), the state will be
 *  changed to #artx_MS_SENT. At the end of the next monitoring
 *  interval, the state of all routines that are in #artx_MS_SENT
 *  state is set back to #artx_MS_COLLECT.
 */
enum artx_monitor_state
{
  artx_MS_COLLECT,               //!< Collecting monitoring information
  artx_MS_READY,                 //!< Information ready to be transmitted
  artx_MS_SENT                   //!< Information has been sent
};

/**
 *  Task monitoring info
 *
 *  \internal
 *
 *  This aggregate contains all monitoring information for
 *  a single task.
 */
struct artx_monitor_task
{
  uint32_t current_cycles;       //!< How many cycles have been spent
  uint32_t peak_cycles;          //!< Peak of current_cycles
  uint32_t total_cycles;         //!< Accumulated cycles of run_counter runs
  uint16_t run_counter;          //!< How many times the task was scheduled
  uint8_t intervals;             //!< Number of intervals used for monitoring
  uint16_t stack_size;           //!< Stack size of task (bytes)
  uint16_t stack_usage;          //!< Used stack size of task (bytes)

  // The following data will not be sent directly
  PGM_P name;                    //!< ASCII name of task/routine
  enum artx_monitor_state state; //!< Monitoring state
  uint8_t *stack_ptr;            //!< Pointer to user stack
};

/**
 *  Routine monitoring info
 *
 *  \internal
 *
 *  This aggregate contains all monitoring information for
 *  a single routine.
 */
struct artx_monitor_rout
{
  uint32_t current_cycles;       //!< how many cycles have been spent
  uint32_t peak_cycles;          //!< peak of current_cycles
  uint32_t total_cycles;         //!< accumulated cycles of run_counter runs
  uint16_t run_counter;          //!< how many times the routine was scheduled
  uint8_t intervals;             //!< number of intervals used for monitoring

  // The following data will not be sent directly
  PGM_P name;                    //!< ASCII name of task/routine
  enum artx_monitor_state state; //!< monitoring state
  uint8_t running;               //!< currently running routine of task
};

/**
 *  Declare task or routine name
 *
 *  \internal
 *  \hideinitializer
 *
 *  This macro declares a string containing the name of a task
 *  or routine in flash memory.
 */
#define artx_NAME_DECL(the_name)                                           \
              static const char the_name ## _name[] PROGMEM = (#the_name);

/**
 *  Task monitoring info initializer
 *
 *  \internal
 *  \hideinitializer
 *
 *  This macro initializes the monitoring info for a task.
 */
#define artx_MONITOR_TASK_INIT_(member, task)                              \
              .member = { .stack_size = sizeof(task ## _stack)             \
                                      - artx_STACK_OVERHEAD,               \
                          .stack_ptr = &task ## _stack[artx_CONTEXT_SIZE], \
                          .intervals = 1,                                  \
                          .state = artx_MS_COLLECT,                        \
                          .name = &task ## _name[0] },

/**
 *  Routine monitoring info initializer
 *
 *  \internal
 *  \hideinitializer
 *
 *  This macro initializes the monitoring info for a routine.
 */
#define artx_MONITOR_ROUT_INIT_(member, rout)                              \
              .member = { .intervals = 1,                                  \
                          .state = artx_MS_COLLECT,                        \
                          .name = &rout ## _name[0] },

/**
 *  Monitor controlling
 *
 *  \internal
 *
 *  This aggregate is used by the kernel to signal the monitor
 *  when to transmit its data. It works very similar to how tasks
 *  are triggered, only that the monitor itself runs in the
 *  background task.
 */
struct artx_monitor_control
{
  uint8_t transmit_request;      //!< The available data should be transmitted
  uint16_t interval;             //!< Monitoring interval in ticks
  uint16_t schedule;             //!< Schedule for monitoring interval
};

extern struct artx_monitor_control artx_monitor_ctl;

void artx_monitor_transmit(void);
void artx_monitor_task_init(struct artx_monitor_task *mon);
void ARTX_monitor_set_interval(uint16_t interval);

#else /* !ARTX_ENABLE_MONITOR */

# define artx_MONITOR_EXTRA_STACK  0

# define artx_NAME_DECL(the_name)
# define artx_MONITOR_TASK_INIT_(member, name_str)
# define artx_MONITOR_ROUT_INIT_(member, name_str)

#endif /* ARTX_ENABLE_MONITOR */

#endif
