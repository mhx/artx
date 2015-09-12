/*******************************************************************************
*
* ARTX task and routine handling
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
 *  \file task.c
 *  \brief Task and routine handling
 */

/**
 *  \mainpage Overview
 *
 *  \section s_start Where it all started...
 *
 *    One evening, when I was a little frustrated after looking for a
 *    small real-time operating system for the AVR microcontrollers,
 *    I said to a colleague via ICQ:
 * 
 *     <em>"Als nächstes muss ich mir erstmal ein kleines Betriebssystem
 *          schreiben. Alles, was ich bisher gesehen habe, ist irgendwie
 *          Murks."</em> (mhx, 2007-02-22)
 *
 *    A bad translation of that would be something like:
 *
 *     <em>"Next, I have to write a little operating system. Everything
 *          I've seen until now is crap."</em>
 *
 *    That more or less was when I started thinking about the implementation
 *    of ARTX.
 *
 *  \section s_intro Introduction
 *
 *    ARTX is a pre-emptive real-time operating system kernel for Atmel's
 *    8-bit AVR microcontrollers. It primary goals are:
 *
 *      - Small memory footprint. The minimal configuration uses
 *        only 420 bytes of flash memory.
 *
 *      - Easy to use C interface.
 *
 *      - ... 
 *
 *    ARTX is the abbreviation for <b>A</b>tmel/<b>A</b>VR <b>R</b>eal
 *    <b>T</b>ime e<b>X</b>ecutive.
 *
 *  \section s_concepts Basic Concepts
 *
 *    The scheduler is driven by a timer interrupt. As soon as
 *    the interrupt fires, we need to save the context of the
 *    currently executing code and switch to kernel context.
 *
 *    The interrupt itself will already push the program counter,
 *    so there's no need to save the PC. It will be restored when
 *    the task is resumed by a RETI.
 *
 *    So, we actually need to:
 *
 *      - save the AVR status register (SREG)
 *
 *      - save all 32 general purpose registers
 *
 *    All the above should be stored on the task's own stack.
 *
 *    So, we actually need to allocate 32 + 1 (SREG) + 2 (PC) + 2 (rout)
 *    extra bytes for each task's stack.
 *
 *    Then we need to:
 *
 *      - store the stack pointer (SP) in the task's control block,
 *        so it can be restored when the task is resumed
 *
 *      - switch to the kernel stack
 *
 *    The minimum RAM overhead for each task is 37 (context) +
 *    11 (TCB) == 48 bytes.
 *
 *    The list of TCBs is always sorted by priority. This makes
 *    it very easy to search for the highest priority task that
 *    needs to be scheduled.
 *
 *      - We setup the whole task together with its running
 *        routine(s) (then we don't have to handle returns
 *        from the task, and we don't have to put the burden
 *        of creating a runloop for each task on the user).
 *
 *      - Routines can be registered to run in each task.
 *        So tasks always run, but routines can be enabled
 *        or disabled.
 *
 *      - When a (timedriven) task has run all of its routines,
 *        it suspends itself and resumes when it is scheduled
 *        again.
 *
 *      - When switching from one task to another by using a
 *        function call instead of a timer interrupt, we need
 *        to simulate an interrupt and, after switching the
 *        stack, just RETI.
 *
 *  \todo
 *
 *   - more intelligent locking?
 *
 *   - don't disable interrupts?
 *
 *   - in TICK, when locked, don't switch task?
 *
 */


/*===== GLOBAL INCLUDES ======================================================*/

#include <avr/io.h>
#include <avr/interrupt.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/task.h"
#include "artx/tick.h"
#include "artx/util.h"
#include "artx/handy.h"
#include "artx/monitor.h"


/*===== DEFINES ==============================================================*/

#define artx_USEC_ONE_SECOND 1000000UL

#define artx_TICK_LENGTH_USEC  ((uint32_t) (((uint64_t) artx_USEC_ONE_SECOND*ARTX_TICK_DURATION*ARTX_TICK_PRESCALER + ARTX_CLOCK_FREQUENCY/2)/ARTX_CLOCK_FREQUENCY))

/**
 *  Check if a routine is currently enabled
 *
 *  \internal
 *  \hideinitializer
 */
#if ARTX_USE_ROUT_STATE
# define artx_ROUT_IS_ENABLED(rcb)   (ARTX_rout_get_state(rcb) == ARTX_RS_ENABLED)
#else
# define artx_ROUT_IS_ENABLED(rcb)   1
#endif

/**
 *  Pop General Purpose Registers
 *
 *  \internal
 *  \hideinitializer
 *
 *  This macro pops most of the general purpose registers
 *  from the stack. This is used by the task switching code
 *  and the main reason for this macro is to keep that code
 *  as simple as possible.
 */
#define artx_POP_R0_THRU_R29                                 \
          "pop  r0 \n\tpop  r1 \n\tpop  r2 \n\tpop  r3 \n\t" \
          "pop  r4 \n\tpop  r5 \n\tpop  r6 \n\tpop  r7 \n\t" \
          "pop  r8 \n\tpop  r9 \n\tpop  r10\n\tpop  r11\n\t" \
          "pop  r12\n\tpop  r13\n\tpop  r14\n\tpop  r15\n\t" \
          "pop  r16\n\tpop  r17\n\tpop  r18\n\tpop  r19\n\t" \
          "pop  r20\n\tpop  r21\n\tpop  r22\n\tpop  r23\n\t" \
          "pop  r24\n\tpop  r25\n\tpop  r26\n\tpop  r27\n\t" \
          "pop  r28\n\tpop  r29\n\t"

/**
 *  Push General Purpose Registers
 *
 *  \internal
 *  \hideinitializer
 *
 *  This macro pushes most of the general purpose registers
 *  to the stack. This is used by the task switching code
 *  and the main reason for this macro is to keep that code
 *  as simple as possible.
 */
#define artx_PUSH_R29_THRU_R0                                \
                                  "push r29\n\tpush r28\n\t" \
          "push r27\n\tpush r26\n\tpush r25\n\tpush r24\n\t" \
          "push r23\n\tpush r22\n\tpush r21\n\tpush r20\n\t" \
          "push r19\n\tpush r18\n\tpush r17\n\tpush r16\n\t" \
          "push r15\n\tpush r14\n\tpush r13\n\tpush r12\n\t" \
          "push r11\n\tpush r10\n\tpush r9 \n\tpush r8 \n\t" \
          "push r7 \n\tpush r6 \n\tpush r5 \n\tpush r4 \n\t" \
          "push r3 \n\tpush r2 \n\tpush r1 \n\tpush r0 \n\t"


/*===== TYPEDEFS =============================================================*/

#if ARTX_ENABLE_MONITOR || ARTX_ENABLE_TICK_SYNC || ARTX_ENABLE_TIME

/**
 *  Timer type
 *
 *  \internal
 *
 *  A type that can hold all possible values of the timer/counter
 *  register used as the tick source. Since multiple tick sources
 *  are supported, this type has a different width depending on
 *  the tick source. See artx/tick.h for tick configuration.
 */
typedef artx_TIMER_TYPE artx_timer_type;

#endif


/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

#if ARTX_ENABLE_MONITOR
static artx_timer_type artx_elapsed(void);
#endif

static artxNORETURN artxNAKED void artx_run_task(void);

static artxNEVERINLINE artxNAKED void artx_yield(void); // TODO: why is this naked?

static artxALWAYSINLINE inline void artx_pop_context(void);
static artxALWAYSINLINE inline void artx_push_context(void);


/*===== EXTERNAL VARIABLES ===================================================*/

/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

/**
 *  Task list
 *
 *  \internal
 *
 *  Pointer to the first element of the task list. The task list is
 *  kept sorted by priority. The first element is the task with the
 *  highest priority, the last element is the idle task.
 */
#if !ARTX_ENABLE_MONITOR
static
#endif
       struct artx_tcb *artx_task_list = 0;

/**
 *  Task control block of the currently running task
 *
 *  \internal
 */
static struct artx_tcb * volatile artx_current_tcb;

/**
 *  Tick indicator
 *
 *  \internal
 *
 *  This variable, when set to a nonzero value, indicates that the
 *  scheduler has been triggered by the tick and not by a yielding
 *  task.
 */
static volatile uint8_t artx_is_tick;

#if ARTX_ENABLE_TIME
/**
 *  Tick indicator
 *
 *  \internal
 *
 *  This variable, when set to a nonzero value, indicates that the
 *  scheduler has been triggered by the tick and not by a yielding
 *  task.
 */
static uint32_t artx_us_time;
static uint32_t artx_us_tmp;
static uint32_t artx_s_time;
#endif

#if ARTX_ENABLE_MONITOR

/**
 *  Last timer/counter value
 *
 *  \internal
 *
 *  This variable holds the value of the timer/counter register
 *  used as the tick source at the time of the last task switch.
 *  It is used to calculate the amount of cycles spent in the
 *  different tasks and routines.
 */
static volatile artx_timer_type artx_last_timer;

static uint8_t artxASMONLY artx_SREG; //!< SREG temporary storage \internal
static uint8_t artxASMONLY artx_R31;  //!< R31 temporary storage \internal
static uint8_t artxASMONLY artx_R30;  //!< R30 temporary storage \internal
static uint8_t artxASMONLY artx_R29;  //!< R29 temporary storage \internal
static uint8_t artxASMONLY artx_R28;  //!< R28 temporary storage \internal

#endif // ARTX_ENABLE_MONITOR

#if ARTX_ENABLE_TICK_SYNC

#if ARTX_ENABLE_MONITOR
/**
 *  Last timer top value
 *
 *  \internal
 *
 *  This is used to correctly calculate the number of cycles
 *  spent in a certain task or routine when tick synchronization
 *  is enabled.
 */
static artx_timer_type artx_last_timer_top = artx_TIMER_TOP;
#endif

/**
 *  Tick synchronization counter
 *
 *  \internal
 *
 *  This counter is decremented with each tick. When it drops to
 *  -#ARTX_SYNC_TICKS/2, it will be reset to +#ARTX_SYNC_TICKS/2.
 *  The closer this counter is to zero when the tick interrupt
 *  arrives, the better the synchronization.
 */
static int16_t artx_sync_ctr = 1;

/**
 *  Tick synchronization correction
 *
 *  \internal
 *
 *  This variable holds the delta that is added on top of the
 *  user-defined #ARTX_TICK_DURATION for synchronization.
 */
static int16_t artx_sync_delta;

/**
 *  Current synchronization status
 *
 *  \internal
 */
static struct ARTX_sync_status artx_sync_status;

#endif // ARTX_ENABLE_TICK_SYNC


/*===== STATIC FUNCTIONS =====================================================*/

#if ARTX_ENABLE_MONITOR

/**
 *  Get time since last task switch
 *
 *  \internal
 *
 *  This routine returns the time that has elapsed since the last
 *  task switch. The unit is determined by the timer used as the
 *  kernel's tick source. If no prescaler is used, the unit is 1
 *  clock cycle. If a prescaler of 64 is used, the unit is 64 cycles.
 *
 *  \returns Time since last task switch in units of the timer used
 *           as the tick source.
 */

static artx_timer_type artx_elapsed(void)
{
  artx_timer_type current = artx_TIMER_REG;

  if (current < artx_last_timer)
  {
#if ARTX_ENABLE_TICK_SYNC
    current += artx_last_timer_top;
#else
    current += artx_TIMER_TOP;
#endif
  }

  return current - artx_last_timer;
}
#endif

/**
 *  Save a task's context
 *
 *  \internal
 *
 *  This routine fully saves a task's context to its stack and
 *  control block.
 */

static inline void artx_push_context(void)
{
  asm volatile (

    "in    r31, __SREG__             \n\t" /* get SREG               */

#if ARTX_ENABLE_MONITOR
    "sts   artx_SREG, r31            \n\t" /* save SREG              */

    "sts   artx_R30, r30             \n\t" /* save R30               */
    "sts   artx_R29, r29             \n\t" /* save R29               */

    "lds   r30, artx_current_tcb     \n\t" /* load pointer to SP     */
    "lds   r31, artx_current_tcb + 1 \n\t" /*   buffer into Z reg    */

    "in    r29, __SP_L__             \n\t" /* save low byte of SP    */
    "st    z+, r29                   \n\t"
    "in    r29, __SP_H__             \n\t" /* save high byte of SP   */
    "st    z+, r29                   \n\t"

    "ld    r29, z+                   \n\t" /* load CXT-SP            */
    "out   __SP_L__, r29             \n\t"
    "ld    r29, z+                   \n\t"
    "out   __SP_H__, r29             \n\t"

    "lds   r31, artx_R31             \n\t" /* load R31               */
    "push  r31                       \n\t" /* save R31               */

    "lds   r31, artx_SREG            \n\t" /* load SREG              */
    "push  r31                       \n\t" /* save SREG              */

    "lds   r30, artx_R30             \n\t" /* load R30               */
    "push  r30                       \n\t" /* save R30               */

    "lds   r29, artx_R29             \n\t" /* load R29               */
#else
    "push  r31                       \n\t" /* save SREG              */
    "push  r30                       \n\t" /* save R30               */
#endif

    artx_PUSH_R29_THRU_R0                  /* save remaining regs    */

    "lds   r26, artx_current_tcb     \n\t" /* load pointer to SP     */
    "lds   r27, artx_current_tcb + 1 \n\t" /*   buffer into X reg    */

#if ARTX_ENABLE_MONITOR
    "adiw  r26, 2                    \n\t" /* adjust for CXT-SP      */
#endif

    "in    r0, __SP_L__              \n\t" /* save low byte of SP    */
    "st    x+, r0                    \n\t"
    "in    r0, __SP_H__              \n\t" /* save high byte of SP   */
    "st    x+, r0                    \n\t"

    "ldi   r28, lo8(__stack)         \n\t" /* switch to kernel stack */
    "ldi   r29, hi8(__stack)         \n\t"
    "out   __SP_L__, r28             \n\t"
    "out   __SP_H__, r29             \n\t"

  );
}

/**
 *  Restore a task's context
 *
 *  \internal
 *
 *  This routine fully restores a task's context and finally
 *  returns to where that task left off when it was interrupted
 *  by the tick or yielded to the scheduler.
 */

static inline void artx_pop_context(void)
{
  asm volatile (

    "lds  r26, artx_current_tcb      \n\t" /* load pointer to SP     */
    "lds  r27, artx_current_tcb + 1  \n\t" /*   buffer into X reg    */

#if ARTX_ENABLE_MONITOR
    "adiw  r26, 2                    \n\t" /* adjust for CXT-SP      */
#endif

    "ld   r0, x+                     \n\t" /* restore stack pointer  */
    "out  __SP_L__, r0               \n\t"
    "ld   r0, x+                     \n\t"
    "out  __SP_H__, r0               \n\t"

    artx_POP_R0_THRU_R29                   /* restore all registers  */

#if ARTX_ENABLE_MONITOR
    "sts artx_R29, r29               \n\t"

    "pop r30                         \n\t"
    "sts artx_R30, r30               \n\t"

    "pop r31                         \n\t"
    "sts artx_SREG, r31              \n\t"

    "pop r31                         \n\t"
    "sts artx_R31, r31               \n\t"

    "lds  r30, artx_current_tcb      \n\t" /* load pointer to CXT-SP */
    "lds  r31, artx_current_tcb + 1  \n\t" /*   buffer into Z reg    */
    "adiw r30, 4                     \n\t"

    "in   r29, __SP_H__              \n\t" /* save context stack ptr */
    "st   -z, r29                    \n\t"
    "in   r29, __SP_L__              \n\t"
    "st   -z, r29                    \n\t"

    "ld   r29, -z                    \n\t" /* restore stack pointer  */
    "out  __SP_H__, r29              \n\t"
    "ld   r29, -z                    \n\t"
    "out  __SP_L__, r29              \n\t"

    "lds r29, artx_R29               \n\t"
    "lds r30, artx_R30               \n\t"

    "lds r31, artx_SREG              \n\t" /* load original SREG     */
    "out __SREG__, r31               \n\t" /* restore SREG           */

    "lds r31, artx_R31               \n\t" /* restore R31            */
#else
    "pop r30                         \n\t"

    "pop r31                         \n\t" /* load original SREG     */
    "out __SREG__, r31               \n\t" /* restore SREG           */

    "pop r31                         \n\t" /* restore R31            */
#endif

    "reti                            \n\t" /* return and enable intr */

  );
}

/**
 *  Yield to the scheduler
 *
 *  \internal
 *
 *  This routine is either triggered by the a task that has performed
 *  all its duties or by the tick interrupt.
 *
 *  This routine has to be a real function, so the return address is
 *  pushed onto the stack when it's called by artx_run_task() because
 *  we're later using a \c RETI to return back to the task. That's why
 *  we declare it to be noinline.
 *
 *  When the routine is called by the tick, the global variable
 *  #artx_is_tick has been set to a nonzero value, and the schedule
 *  of all TCBs is decremented.
 */

static void artx_yield(void)
{
  asm volatile (

#if ARTX_ENABLE_MONITOR
    "sts   artx_R31, r31             \n\t" /* save R31               */
#else
    "push  r31                       \n\t" /* save R31               */
#endif

    "ldi   r31, 0                    \n\t"
    "sts   artx_is_tick, r31         \n\t"

    "artx_do_yield:                  \n\t"

  );

  artx_push_context();

  /*
   *  gcc expects the __zero_reg__ to be zero, but the task we've just
   *  interrupted may have temporarily clobbered that register, so we
   *  need to restore it just in case it is used by the kernel code.
   *  Since we've saved the original __zero_reg__ contents on the stack,
   *  we make sure that the clobbered register will be correctly restored.
   */
  asm volatile ("clr __zero_reg__");

  if (artx_is_tick)
  {
#if ARTX_ENABLE_TIME
    artx_us_tmp += artx_TICK_LENGTH_USEC;
    artx_us_time += artx_TICK_LENGTH_USEC;

    while (artx_us_tmp >= artx_USEC_ONE_SECOND)
    {
      artx_us_tmp -= artx_USEC_ONE_SECOND;
      artx_s_time++;
    }
#endif

#if ARTX_ENABLE_MONITOR
    if (artx_current_tcb->mon.state == artx_MS_COLLECT)
    {
      artx_current_tcb->mon.current_cycles += artx_elapsed();
    }
#endif

    for (register struct artx_tcb *tcb = artx_task_list; tcb; tcb = tcb->next)
    {
      if (artxLIKELY(tcb->schedule > -32768))
      {
        tcb->schedule--;
      }
    }

#if ARTX_ENABLE_TICK_SYNC

#if ARTX_ENABLE_MONITOR
    artx_last_timer_top = artx_CUR_TIMER_TOP;
#endif

    artx_TICK_ADJUST(artx_sync_delta);

    if (--artx_sync_ctr == -ARTX_SYNC_TICKS/2)
    {
      artx_sync_ctr = ARTX_SYNC_TICKS/2;
    }
#endif

#if ARTX_ENABLE_MONITOR
    if (artxLIKELY(artx_monitor_ctl.schedule > 0))
    {
      if (artxUNLIKELY(--artx_monitor_ctl.schedule == 0))
      {
        for (register struct artx_tcb *tcb = artx_task_list; tcb; tcb = tcb->next)
        {
          switch (tcb->mon.state)
          {
            case artx_MS_COLLECT:
              if (artxLIKELY(tcb->mon.run_counter > 0))
              {
                tcb->mon.state = artx_MS_READY;
              }
              else
              {
                tcb->mon.intervals++;
              }
              break;

            case artx_MS_SENT:
              tcb->mon.current_cycles = 0;
              tcb->mon.state = artx_MS_COLLECT;
              break;

            default:
              break;
          }

#if ARTX_USE_MULTI_ROUT
          for (register struct artx_rcb *rcb = tcb->rout; rcb; rcb = rcb->next)
          {
            switch (rcb->mon.state)
            {
              case artx_MS_COLLECT:
                if (artxLIKELY(rcb->mon.run_counter > 0))
                {
                  if (artxUNLIKELY(rcb->mon.running))
                  {
                    rcb->mon.current_cycles += tcb->mon.current_cycles;
                  }

                  rcb->mon.state = artx_MS_READY;
                }
                else
                {
                  rcb->mon.intervals++;
                }
                break;

              case artx_MS_SENT:
                rcb->mon.current_cycles = 0;
                rcb->mon.state = artx_MS_COLLECT;
                break;

              default:
                break;
            }
          }
#endif
        }

        artx_monitor_ctl.schedule = artx_monitor_ctl.interval;
        artx_monitor_ctl.transmit_request = 1;
      }
    }
#endif
  }

  asm volatile ("rjmp artx_task_switch");
}

/**
 *  Main task routine
 *
 *  \internal
 *
 *  This routine is the entry point for every task. It implements
 *  a simple loop that runs all enabled routines that have been
 *  registered for the task and then yields to the scheduler.
 *
 *  The routine also contains most of the monitoring logic.
 *
 *  The routine never returns.
 */

static void artx_run_task(void)
{
  /*
   *  Registers used in this routine are protected by the task
   *  switching logic. Routines called from here must push
   *  the registers they use, and upon context switch, all
   *  registers are saved. So we only need to fetch the
   *  pointer to the current task once.
   */

  ARTX_disable_int();

  register struct artx_tcb *tcb = artx_current_tcb;

  ARTX_enable_int();

  for (;;)
  {
#if ARTX_USE_MULTI_ROUT

    for (register struct artx_rcb *p = tcb->rout; p; p = p->next)
    {
      if (artxLIKELY(artx_ROUT_IS_ENABLED(p)))
      {
#if ARTX_ENABLE_MONITOR
        ARTX_disable_int();

        if (p->mon.state == artx_MS_COLLECT)
        {
          p->mon.current_cycles = -(tcb->mon.current_cycles + artx_elapsed());
          p->mon.running = 1;
        }

        ARTX_enable_int();
#endif

        p->rout();

#if ARTX_ENABLE_MONITOR
        ARTX_disable_int();

        if (p->mon.state == artx_MS_COLLECT)
        {
          p->mon.running = 0;
          p->mon.run_counter++;
          p->mon.current_cycles += tcb->mon.current_cycles;
          p->mon.current_cycles += artx_elapsed();

          if (p->mon.current_cycles > p->mon.peak_cycles)
          {
            p->mon.peak_cycles = p->mon.current_cycles;
          }

          p->mon.total_cycles += p->mon.current_cycles;
          p->mon.current_cycles = 0;
        }

        ARTX_enable_int();
#endif
      }
    }

#else /* !ARTX_USE_MULTI_ROUT */

    tcb->rout();

#endif /* !ARTX_USE_MULTI_ROUT */

    /* artx_yield() requires us to disable interrupts */
    asm volatile ("cli");

    tcb->schedule += tcb->interval;

#if ARTX_ENABLE_MONITOR

    if (tcb->mon.state == artx_MS_COLLECT)
    {
      tcb->mon.run_counter++;
      tcb->mon.current_cycles += artx_elapsed();

      if (tcb->mon.current_cycles > tcb->mon.peak_cycles)
      {
        tcb->mon.peak_cycles = tcb->mon.current_cycles;
      }

      tcb->mon.total_cycles += tcb->mon.current_cycles;
      tcb->mon.current_cycles = 0;
    }

    if (artxUNLIKELY(tcb->priority == artx_PRIO_IDLE) &&
        artxUNLIKELY(artx_monitor_ctl.transmit_request))
    {
      artx_monitor_ctl.transmit_request = 0;
      artx_monitor_transmit();
    }

#endif

    /* interrupts have already been disabled above */
    artx_yield();
  }
}


/*===== FUNCTIONS ============================================================*/

/**
 *  Initialize Task
 *
 *  This routine initializes both the task control block as well as
 *  the tasks stack. It also adds the task to the kernel's task list
 *  so it will be scheduled when the kernel is running.
 *
 *  \param tcb                   Pointer to the task control block.
 */

void ARTX_task_init(struct artx_tcb *tcb)
{
#if ARTX_ENABLE_MONITOR
  artx_monitor_task_init(&tcb->mon);
#endif

  uint8_t *sp = (uint8_t *) tcb->sp;
  uint16_t raddr = (uint16_t) &artx_run_task;

  /* initialize stack by simulating a context push */

  *sp-- = raddr & 0xFF;    /* return address low byte   */
  *sp-- = raddr >> 8;      /* return address high byte  */

#if ARTX_ENABLE_MONITOR
  /* switch from user stack to context stack */
  tcb->sp = (uint16_t) sp;

  sp = (uint8_t *) tcb->sp_cxt;
#endif

  for (uint8_t r = 33; r--; )
  {
    *sp-- = 0;             /* R31, SREG, R30 .. R0      */
  }

#if ARTX_ENABLE_MONITOR
  tcb->sp_cxt = (uint16_t) sp;
#else
  tcb->sp = (uint16_t) sp;
#endif

  /* sort tasks by priority */

  struct artx_tcb **pp = &artx_task_list;

  while (*pp && tcb->priority >= (*pp)->priority)
  {
    pp = &(*pp)->next;
  }

  tcb->next = *pp;
  *pp = tcb;
}

#if ARTX_USE_MULTI_ROUT

/**
 *  Push routine on a tasks run queue
 *
 *  Call this routine to push a routine allocated using #ARTX_ROUT on
 *  the run queue of a task. The routine will be added to the end of
 *  the run queue. Note that unless #ARTX_USE_MULTI_ROUT is defined to 1,
 *  only one routine may be pushed and consecutive calls will silently
 *  override the previous calls.
 *
 *  The same routine cannot be pushed pushed on the run queue of
 *  different tasks.
 *
 *  \param tcb                   Pointer to the task control block.
 *
 *  \param rout                  Pointer to the routine control block.
 */

void ARTX_task_push_rout(struct artx_tcb *tcb, struct artx_rcb *rout)
{
  rout->next = 0;

  struct artx_rcb **pp = &tcb->rout;

  while (*pp)
  {
    pp = &(*pp)->next;
  }

  *pp = rout;
}

#endif

/**
 *  Run the scheduler
 *
 *  This routine contains the code for the scheduler. The code is
 *  kept very simple to make sure task switching is fast.
 *
 *  Call this routine after you've set up all tasks. Make sure you
 *  also call #ARTX_TICK_INIT before to initialize the tick source.
 *
 *  The routine never returns. (Unfortunately, it cannot be marked
 *  as \c noreturn as it will actually return to the current task.)
 */

void ARTX_schedule(void)
{
  asm volatile ("artx_task_switch:");

  register struct artx_tcb *tcb = artx_task_list;

  /*
   *  TODO: recalculate
   *
   *   ====> (34 .. 21 + 13*num_of_tasks)  +  7 if task switch
   *         + (9 + 12*num_of_tasks) if tick
   *
   *   ====> 4 tasks:  34 .. 80   mean: 57
   *
   *   Full switch: PUSH 100 + SWITCH 57 + POP 81 => 238
   *
   *         Yield: 215 .. 261   (238)   (~12us @ 20MHz)
   *          Tick: 272 .. 318   (295)   (~15us @ 20MHz)
   *
   *   Since usually high priority tasks are scheduled more
   *   frequently,
   *
   *   21 cycles can be saved per task switch by using the
   *   fast context push code. (At the expense of increasing
   *   the code size by 40 bytes.)
   */

  /*
   *  TODO: comment is not accurate
   *  *ONLY* the task priority is relevant when determining
   *  which task should be scheduled next. Multiple tasks
   *  with the same priority that are ready to be scheduled
   *  are scheduled in arbitrary order.
   */

  while (tcb->schedule > 0)
  {
    tcb = tcb->next;
  }

  // if (tcb == 0)
  // {
  //   /* TODO: this is an error (do we need a backup idle task?) */
  //   /* only need to check this if tasks can be terminated? */
  // }

  /*
   *  If the same task is about to be run again, there's no
   *  need to switch at all.
   */
  if (tcb != artx_current_tcb)
  {
    artx_current_tcb = tcb;
  }

#if ARTX_ENABLE_MONITOR
  artx_last_timer = artx_TIMER_REG;
#endif

  artx_pop_context();
}

#ifdef artx_TICK_VECTOR

/**
 *  ARTX Kernel Tick
 *
 *  \internal
 *
 *  This routine is triggered by the selected tick source and
 *  initiates a task switch.
 */

ISR(artx_TICK_VECTOR, ISR_NAKED)
{
  asm volatile (
#if ARTX_ENABLE_MONITOR
    "sts   artx_R31, r31             \n\t" /* save R31               */
#else
    "push  r31                       \n\t" /* save R31               */
#endif

    "ldi   r31, 1                    \n\t"
    "sts   artx_is_tick, r31         \n\t"

    "rjmp  artx_do_yield             \n\t"
  );
}

#else

# error "no artx_TICK_VECTOR is set"

#endif

#if ARTX_ENABLE_TICK_SYNC

/**
 *  ARTX Kernel Tick Synchronization
 *
 *  This routine can be used to synchronize the scheduler
 *  with some external, highly accurate, low-frequency time
 *  source. For example, if you have a precise 1 second
 *  interrupt available, set ARTX_SYNC_TICKS to the number
 *  of kernel ticks per seconds and call this function
 *  whenever the interrupt occurs.
 *
 *  Calls to this routine have to be locked.
 */

void ARTX_tick_sync(void)
{
  /*
   * TODO: This expression needs to be optimized!
   *       Currently, this needs more than 700 cycles to execute.
   *       It really should not need more than 100-200 cycles.
   */

  int16_t sync_ctr = artx_sync_ctr;
  artx_timer_type timer_val = artx_TIMER_REG;

  artx_sync_status.sync_ctr = sync_ctr;
  artx_sync_status.timer_val = timer_val;

  int32_t d32 = ((int32_t) sync_ctr*ARTX_TICK_DURATION - timer_val)/ARTX_SYNC_TICKS;
  int16_t delta;

  if (d32 > ARTX_MAX_SYNC_ADJUST)
  {
    delta = ARTX_MAX_SYNC_ADJUST;
  }
  else if (d32 < -ARTX_MAX_SYNC_ADJUST)
  {
    delta = -ARTX_MAX_SYNC_ADJUST;
  }
  else
  {
    delta = (int16_t) d32;
  }

#if 0
#define ARTX_TS_MEAN_BUF_BITS 2

#if ARTX_TS_MEAN_BUF_BITS
#define artx_TS_MEAN_BUF_SIZE (1 << ARTX_TS_MEAN_BUF_BITS)
  static int32_t accu;
  static int16_t buf[artx_TS_MEAN_BUF_SIZE];
  static uint8_t first, last;

  if (++first == artx_TS_MEAN_BUF_SIZE) first = 0;

  if (first == last)
  {
    accu -= buf[last];
    if (++last == artx_TS_MEAN_BUF_SIZE) last = 0;
  }

  buf[first] = delta;
  accu += delta;

  delta = accu >> ARTX_TS_MEAN_BUF_BITS;
#endif
#endif

  artx_sync_delta = -delta;
  artx_sync_status.correction = artx_sync_delta;
}

/**
 *  Get Kernel Tick Synchronization Status
 *
 *  This routine can be used to retrieve information about
 *  the status of the tick synchronization, should this
 *  ever be required.
 *
 *  Calls to this routine must be locked.
 *
 *  \param status                Pointer to a buffer to store
 *                               the status information.
 */

void ARTX_get_sync_status(struct ARTX_sync_status *status)
{
  *status = artx_sync_status;
}

#endif

#if ARTX_ENABLE_TIME

/**
 *  Microseconds since last Kernel Tick
 *
 *  \internal
 *
 *  This routine can be used to get the number of
 *  microseconds elapsed since the last kernel tick.
 *
 *  Calls to this routine must be locked.
 *
 *  \returns Number of microseconds since last kernel tick.
 */

static inline uint32_t artx_usec_since_last_tick(void)
{
  return (artx_TIMER_REG*(uint32_t) (((uint64_t) (1UL << 8)*artx_USEC_ONE_SECOND*ARTX_TICK_PRESCALER
                                              + ARTX_CLOCK_FREQUENCY/2)/ARTX_CLOCK_FREQUENCY)) >> 8;
}

/**
 *  High resolution time information
 *
 *  This routine can be used to retrieve the time since the
 *  kernel started in microsecond resolution.
 *
 *  This time will wrap after about 136 years.
 *
 *  Calls to this routine must be locked.
 *
 *  \param p_time                Pointer to a structure to
 *                               store the time information.
 */

void ARTX_hires_time(struct ARTX_timeval *p_time)
{
  uint32_t usec = artx_us_tmp + artx_usec_since_last_tick();
  uint32_t sec = artx_s_time;

  while (usec >= artx_USEC_ONE_SECOND)
  {
    usec -= artx_USEC_ONE_SECOND;
    sec++;
  }

  p_time->seconds = sec;
  p_time->microseconds = usec;
}

/**
 *  High resolution time difference
 *
 *  Use this routine to calculate the delta between two
 *  times retrieved by ARTX_hires_time().
 *
 *  \param p_t0                  Pointer to the first time
 *                               structure.
 *
 *  \param p_t1                  Pointer to the second time
 *                               structure.
 *
 *  \param p_delta               Pointer to the time difference
 *                               structure.
 */

void ARTX_delta_time(const struct ARTX_timeval *p_t0, const struct ARTX_timeval *p_t1, struct ARTX_timeval *p_delta)
{
  p_delta->seconds = p_t1->seconds - p_t0->seconds;
  p_delta->microseconds = p_t1->microseconds - p_t0->microseconds;

  if (p_t1->microseconds < p_t0->microseconds)
  {
    p_delta->seconds--;
    p_delta->microseconds += artx_USEC_ONE_SECOND;
  }
}

/**
 *  Second resolution time
 *
 *  This routine can be used to retrieve the time since the
 *  kernel started in second resolution.
 *
 *  This time will wrap after about 136 years.
 *
 *  Calls to this routine must be locked.
 *
 *  \returns Number of seconds since the kernel started.
 */

uint32_t ARTX_time(void)
{
  struct ARTX_timeval t;
  ARTX_hires_time(&t);
  return t.seconds;
}

/**
 *  Microsecond resolution time
 *
 *  This routine can be used to retrieve the time since the
 *  kernel started in microsecond resolution.
 *
 *  This time will wrap after about 71 minutes, so it is not
 *  useful for absolute time measurements. Its primary purpose
 *  is to have a fast, high resolution timer for relative time
 *  measurements.
 *
 *  Calls to this routine must be locked.
 *
 *  \returns Number of microseconds since the kernel started.
 */

uint32_t ARTX_us_time(void)
{
  return artx_us_time + artx_usec_since_last_tick();
}

#endif

