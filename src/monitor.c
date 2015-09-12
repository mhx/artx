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
 *  \file monitor.c
 *  \brief Monitoring support
 */


/*===== GLOBAL INCLUDES ======================================================*/

#include <stddef.h>
#include <avr/io.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/monitor.h"
#include "artx/serial.h"
#include "artx/task.h"
#include "artx/tick.h"

#if ARTX_ENABLE_MONITOR


/*===== DEFINES ==============================================================*/

/**
 *  Stack sentinel
 *
 *  \internal
 *
 *  This byte is used to identify unused bytes on the stack.
 */
#define artx_STACK_SENTINEL 0xC3


/*===== TYPEDEFS =============================================================*/

/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

static void update_stack(struct artx_monitor_task *mon);


/*===== EXTERNAL VARIABLES ===================================================*/

extern struct artx_tcb *artx_task_list;


/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

/**
 *  ARTX serial frame marker
 *
 *  \internal
 *
 *  This marker is sent at the beginning of each data frame to
 *  synchronize the serial stream.
 */
static const char artx_marker[] PROGMEM = "ARTX";

/**
 *  Monitor control data
 *
 *  \internal
 */
struct artx_monitor_control artx_monitor_ctl;


/*===== STATIC FUNCTIONS =====================================================*/

/**
 *  Update stack information
 *
 *  \internal
 *
 *  This routine determines the a task's stack usage and updates
 *  the monitoring info accordingly.
 *
 *  \param mon                   Pointer to task monitoring data.
 */

static void update_stack(struct artx_monitor_task *mon)
{
  uint16_t n = mon->stack_size + artx_EXTRA_STACK;
  uint8_t *p = mon->stack_ptr;

  while (*p++ == artx_STACK_SENTINEL && --n > 0)
  { }

  mon->stack_usage = n - artx_TASK_EXTRA_STACK;
}


/*===== FUNCTIONS ============================================================*/

/**
 *  Initialize task monitoring data
 *
 *  \internal
 *
 *  This routine initializes the monitoring data for a task.
 *  Currently, it will only fill the task's stack with sentinel bytes.
 *
 *  \param mon                   Pointer to task monitoring data.
 */

void artx_monitor_task_init(struct artx_monitor_task *mon)
{
  uint16_t i = mon->stack_size + artx_EXTRA_STACK;
  uint8_t *p = mon->stack_ptr;

  while (i-- > 0)
  {
    *p++ = artx_STACK_SENTINEL;
  }
}

/**
 *  Transmit monitoring info
 *
 *  \internal
 *
 *  This routine transmits the monitoring infomation for all tasks
 *  and routines that are in #artx_MS_READY state.
 */

void artx_monitor_transmit(void)
{
  static struct artx_monitor_header header;

  header.version = artx_MONITOR_VERSION;
  header.hdr_size = sizeof(struct artx_monitor_header);
  header.tcb_size = offsetof(struct artx_tcb, mon.name);
#if ARTX_USE_MULTI_ROUT
  header.rcb_size = offsetof(struct artx_rcb, mon.name);
#else
  header.rcb_size = 0;
#endif
  header.nom_tick_duration = artx_TIMER_TOP;
#ifdef artx_CUR_TIMER_TOP
  header.cur_tick_duration = artx_CUR_TIMER_TOP;
#else
  header.cur_tick_duration = artx_TIMER_TOP;
#endif
  header.tick_prescaler = ARTX_TICK_PRESCALER;
  header.monitor_interval = artx_monitor_ctl.interval;
  header.clock_frequency = ARTX_CLOCK_FREQUENCY;

#if ARTX_ENABLE_SERIAL
  ARTX_serial_tx_string_pgm(artx_marker);
  ARTX_serial_tx_data(&header, sizeof(struct artx_monitor_header));
#endif

  register struct artx_tcb *tcb = artx_task_list;

  while (tcb)
  {
    if (tcb->mon.state == artx_MS_READY)
    {
      update_stack(&tcb->mon);

#if ARTX_ENABLE_SERIAL
      ARTX_serial_tx_byte('T');
      ARTX_serial_tx_data(tcb, offsetof(struct artx_tcb, mon.name));
      ARTX_serial_tx_string_pgm(tcb->mon.name);
      ARTX_serial_tx_byte('\0');
#endif

      /* reset content */
      tcb->mon.run_counter = 0;
      tcb->mon.peak_cycles = 0;
      tcb->mon.total_cycles = 0;
      tcb->mon.intervals = 1;
      tcb->mon.state = artx_MS_SENT;

#if ARTX_USE_MULTI_ROUT
      register struct artx_rcb *rcb = tcb->rout;

      while (rcb)
      {
        if (rcb->mon.state == artx_MS_READY)
        {
#if ARTX_ENABLE_SERIAL
          ARTX_serial_tx_byte('R');
          ARTX_serial_tx_data(rcb, offsetof(struct artx_rcb, mon.name));
          ARTX_serial_tx_string_pgm(rcb->mon.name);
          ARTX_serial_tx_byte('\0');
#endif

          /* reset content */
          rcb->mon.run_counter = 0;
          rcb->mon.peak_cycles = 0;
          rcb->mon.total_cycles = 0;
          rcb->mon.intervals = 1;
          rcb->mon.state = artx_MS_SENT;
        }

        rcb = rcb->next;
      }
#endif
    }

    tcb = tcb->next;
  }

#if ARTX_ENABLE_SERIAL
  ARTX_serial_tx_byte('E');
#endif
}

/**
 *  Set monitoring interval
 *
 *  Use this routine to set the monitoring interval. The initial
 *  interval is 0, which means no monitoring data is sent. Data
 *  is updated for each task or routine at every second interval
 *  boundary.
 *
 *  \param interval              Monitoring interval in ticks.
 */

void ARTX_monitor_set_interval(uint16_t interval)
{
  artx_monitor_ctl.interval = interval;
  artx_monitor_ctl.schedule = interval;
}

#endif
