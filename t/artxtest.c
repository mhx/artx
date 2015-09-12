/*******************************************************************************
*
* ARTX test code
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

#include <avr/io.h>

#include "artx/task.h"
#include "artx/tick.h"
#include "artx/serial.h"
#include "artx/monitor.h"

ARTX_TASK(intr,   0,   1, 12); //  2 ms
ARTX_TASK(ut0,    1,   4, 16); //  8 ms
ARTX_TASK(ut1,    2,  25, 16); // 50 ms
ARTX_TASK(ut2,    3,  16, 14); // 32 ms
ARTX_TASK(ut3,    4,  32, 14); // 64 ms
ARTX_IDLE_TASK(idle, 20);

void eat_it(uint8_t task, uint16_t loop) __attribute__((noinline));

void eat_cycles(uint8_t task, uint16_t num) __attribute__((noinline));

void eat_it(uint8_t task, uint16_t loop)
{
  (volatile void) task;
  (volatile void) loop;
  asm volatile ("nop");
}

void eat_cycles(uint8_t task, uint16_t num)
{
  uint16_t i = 0;

  while (i < num)
  {
    eat_it(task, i++);
  }
}

ARTX_ROUT(run_intr)
{
  eat_cycles(0, 4);
}

ARTX_ROUT(run_ut0)
{
  eat_cycles(1, 10);
}

ARTX_ROUT(run_ut1)
{
  eat_cycles(2, 20);
}

#if ARTX_USE_MULTI_ROUT
ARTX_ROUT(more_complex)
{
  eat_cycles(2, 30);
}

ARTX_ROUT(less_complex)
{
  eat_cycles(2, 10);
}
#endif

ARTX_ROUT(run_ut2)
{
  eat_cycles(3, 20);
}

ARTX_ROUT(run_ut3)
{
  eat_cycles(4, 20);
}

ARTX_ROUT(background)
{
  eat_cycles(5, 20);
}

int main(void)
{
#if ARTX_ENABLE_SERIAL
  ARTX_serial_init(115200, 8, 1, ARTX_SP_NONE, ARTX_SM_RW | ARTX_SM_DOUBLE_SPEED);
#endif

#if ARTX_ENABLE_MONITOR
  ARTX_monitor_set_interval(1024);  // every 2 seconds
#endif

  ARTX_task_init(&intr);
  ARTX_task_init(&ut0);
  ARTX_task_init(&ut1);
  ARTX_task_init(&ut2);
  ARTX_task_init(&ut3);
  ARTX_task_init(&idle);

  ARTX_task_push_rout(&intr, &run_intr);
  ARTX_task_push_rout(&ut0, &run_ut0);
  ARTX_task_push_rout(&ut1, &run_ut1);
#if ARTX_USE_MULTI_ROUT
  ARTX_task_push_rout(&ut1, &more_complex);
  ARTX_task_push_rout(&ut1, &less_complex);
#endif
  ARTX_task_push_rout(&ut2, &run_ut2);
  ARTX_task_push_rout(&ut3, &run_ut3);
  ARTX_task_push_rout(&idle, &background);

#if ARTX_USE_ROUT_STATE
  ARTX_rout_enable(&run_intr);
  ARTX_rout_enable(&run_ut0);
  ARTX_rout_enable(&run_ut1);
#if ARTX_USE_MULTI_ROUT
  ARTX_rout_enable(&more_complex);
  ARTX_rout_enable(&less_complex);
#endif
  ARTX_rout_enable(&run_ut2);
  ARTX_rout_enable(&run_ut3);
  ARTX_rout_enable(&background);
#endif

  ARTX_TICK_INIT;

  ARTX_schedule();

  return 0;
}
