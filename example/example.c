/*******************************************************************************
*
* ARTX example project
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

#include "artx/sleep.h"
#include "artx/task.h"
#include "artx/tick.h"

// These definitions are for the LeoStick
#define DDR_LED_R   DDRC
#define DDR_LED_G   DDRB
#define DDR_LED_B   DDRB
#define PORT_LED_R  PORTC
#define PORT_LED_G  PORTB
#define PORT_LED_B  PORTB
#define LED_R  PC7  /* D13 */
#define LED_G  PB5  /* D9  */
#define LED_B  PB6  /* D10 */

ARTX_TASK(fast, 0,  20, 8); //  200 ms
ARTX_TASK(slow, 1, 100, 8); // 1000 ms
ARTX_IDLE_TASK(idle, 8);

ARTX_ROUT(run_slow)
{
  PORT_LED_R ^= 1 << LED_R;
}

ARTX_ROUT(run_fast)
{
  PORT_LED_G ^= 1 << LED_G;
}

ARTX_ROUT(run_idle)
{
  ARTX_sleep(5);
  PORT_LED_B ^= 1 << LED_B;
}

int main(void)
{
  DDR_LED_R |= 1 << LED_R;
  DDR_LED_G |= 1 << LED_G;
  DDR_LED_B |= 1 << LED_B;

  ARTX_task_init(&fast);
  ARTX_task_init(&slow);
  ARTX_task_init(&idle);

  ARTX_task_push_rout(&fast, &run_fast);
  ARTX_task_push_rout(&slow, &run_slow);
  ARTX_task_push_rout(&idle, &run_idle);

  ARTX_TICK_INIT;

  ARTX_schedule();

  return 0;
}
