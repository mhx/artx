#ifndef artx_TASK_H_
#define artx_TASK_H_

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
 *  \file artx/task.h
 *  \brief Task and routine handling
 */

#include <avr/io.h>

#include "artx/artx.h"
#include "artx/handy.h"
#include "artx/monitor.h"

#if ARTX_USE_MULTI_ROUT

/**
 *  Routine State
 *
 *  The run-time state of the routine when ARTX is built with
 *  \c ARTX_USE_ROUT_STATE.
 */
enum ARTX_rout_state
{
  ARTX_RS_DISABLED,              //!< The routine will not be run
  ARTX_RS_ENABLED                //!< The routine will be run
};

/**
 *  Routine Control Block
 *
 *  \internal
 *
 *  A routine control block (RCB) holds all per-routine information.
 *  RCBs are only neccessary if
 *
 *  - multiple routines should be run per task (#ARTX_USE_MULTI_ROUT),
 *  - monitoring support is enabled (#ARTX_ENABLE_MONITOR) or
 *  - you want to enable or disable certain routines at run-time
 *    (#ARTX_USE_ROUT_STATE).
 *
 *  A plain RCB uses only 4 bytes of RAM per routine. Using routine
 *  states adds an extra byte, and monitoring support uses a whole
 *  bunch of memory anyway.
 */
struct artx_rcb
{
  void (*rout)(void);            //!< Address of the routine to execute
  struct artx_rcb *next;         //!< Pointer to next RCB
#if ARTX_USE_ROUT_STATE
  enum ARTX_rout_state state;    //!< Routine state
#endif
#if ARTX_ENABLE_MONITOR
  struct artx_monitor_rout mon;  //!< Routine monitoring info
#endif
};

#endif /* ARTX_USE_MULTI_ROUT */

/**
 *  Task Control Block
 *
 *  \internal
 *
 *  A task control block (TCB) holds all per-task information, like
 *  scheduling information or where the tasks stack is located.
 *
 *  A plain TCB uses 11 bytes of RAM per task. Again, enabling
 *  monitoring support will increase that size. Keep in mind that
 *  the overhead for a task is not only its TCB. Each task has its
 *  own stack frame, and the overhead for storing each task's context
 *  on the stack is a lot larger than the TCB.
 */
struct artx_tcb
{
  volatile uint16_t sp;          //!< Current stack pointer (must be first!)
#if ARTX_ENABLE_MONITOR
  volatile uint16_t sp_cxt;      //!< Context stack pointer (must be second!)
#endif
  struct artx_tcb *next;         //!< Pointer to next task
#if ARTX_USE_MULTI_ROUT
  struct artx_rcb *rout;         //!< Pointer to routines
#else
  void (*rout)(void);            //!< Routine address
#endif
  int16_t schedule;              //!< When the task is about to be scheduled
  int16_t interval;              //!< Multiple of timebase
  uint8_t priority;              //!< 0 - highest / 255 - lowest
#if ARTX_ENABLE_MONITOR
  struct artx_monitor_task mon;  //!< Task monitoring info
#endif
};

#if ARTX_ENABLE_TICK_SYNC
/**
 *  Tick synchronization status
 *
 *  This structure holds information about the current status
 *  of kernel tick synchronization.
 *
 *  This is pretty close to the kernel's internals, so all you
 *  usually need to know is that when the \c sync_ctr and the
 *  \c correction values are close to zero, the kernel is pretty
 *  well synchronized.
 *
 *  \see ARTX_get_sync_status()
 */
struct ARTX_sync_status
{
  int16_t sync_ctr;    //!< Sync counter at time of last tick sync
  uint16_t timer_val;  //!< Timer value at time of last tick sync
  int16_t correction;  //!< Current correction value
};
#endif

#if ARTX_ENABLE_TIME
/**
 *  High resolution time
 *
 *  This structure holds time information with microsecond
 *  resolution.
 */
struct ARTX_timeval
{
  uint32_t seconds;         //!< Seconds
  uint32_t microseconds;    //!< Microseconds
};
#endif

/**
 *  Context Stack Pointer Initializer
 *
 *  \internal
 *  \hideinitializer
 *
 *  When monitoring support is enabled, the task context information
 *  is stored in a different location on the stack to be able to
 *  monitor stack usage accurately. This macro holds the initializer
 *  for the context stack pointer.
 *
 *  \param task                  Task name.
 */
#if ARTX_ENABLE_MONITOR
# define artx_SP_CXT_INIT_(task) .sp_cxt = (uint16_t) &task ## _stack      \
                                               [artx_CONTEXT_SIZE - 1],
#else
# define artx_SP_CXT_INIT_(task)
#endif

/**
 *  Routine State Initializer
 *
 *  \internal
 *  \hideinitializer
 */
#if ARTX_USE_ROUT_STATE
# define artx_INITIAL_ROUT_STATE_    .state = ARTX_RS_DISABLED,
#else
# define artx_INITIAL_ROUT_STATE_
#endif

/**
 *  Stack size required for Task Context
 *
 *  \internal
 *
 *  This defines the amount of bytes required on the stack to save
 *  the full context of a single task. The context consists of all
 *  32 general purpose registers plus the status register.
 */
#define artx_CONTEXT_SIZE      (32 + 1)

/**
 *  Extra stack size required for each task
 *
 *  \internal
 *
 *  The amount of extra bytes required on the stack for each task.
 *  That's two bytes for the return address of the routine that's
 *  being run in the task plus two bytes for the return address
 *  of an interrupt that may occur when the task is being run.
 *
 */
#define artx_TASK_EXTRA_STACK   (2 + 2)

/**
 *  Extra stack size allocated for each task
 *
 *  \internal
 *  \hideinitializer
 *
 *  The amount of extra bytes that is allocated on the stack for
 *  each task. When monitoring support is enabled, some extra
 *  bytes are allocated on the stack to spot stack overruns.
 */
#define artx_EXTRA_STACK  (artx_MONITOR_EXTRA_STACK + artx_TASK_EXTRA_STACK)

/**
 *  Total Stack Overhead for each task
 *
 *  \internal
 *  \hideinitializer
 *
 *  The total number bytes allocated on the stack for each task
 *  on top of the user defined stack size. Without monitoring
 *  support, this is currently 37 bytes.
 */
#define artx_STACK_OVERHEAD  (artx_CONTEXT_SIZE + artx_EXTRA_STACK)

/**
 *  Maximum user task priority
 *
 *  The maximum priority that can be assigned to a user task.
 *  User task priorities start at 0 (highest priority) and range
 *  up to #ARTX_PRIO_USER_MAX (lowest priority).
 */
#define ARTX_PRIO_USER_MAX      223

/**
 *  User task priority offset
 *
 *  \internal
 *
 *  The offset at which user task priorities start. All priorities
 *  below that are reserved for operating system use.
 */
#define artx_PRIO_USER_OFFSET    16

/**
 *  Idle task priority
 *
 *  \internal
 *
 *  The priority at which the idle task is running. This is the
 *  lowest priority possible.
 */
#define artx_PRIO_IDLE          255

/**
 *  Allocate Task
 *
 *  \internal
 *  \hideinitializer
 *
 *  This macro will allocate all resources required for a new task.
 *
 *  \param task                  The unique name of the task.
 *
 *  \param prio                  The unique priority of the task.
 *
 *  \param ival                  The scheduling interval in multiples
 *                               of the tick interval.
 *
 *  \param stack_size            The user stack size in bytes. The stack
 *                               overhead required by the kernel will be
 *                               added automatically.
 *
 *  \param offset                The scheduling offset in multiples
 *                               of the tick interval.
 */
#define artx_ALLOC_TASK(task, prio, ival, stack_size, offset)              \
        ARTX_STATIC_ASSERT((int16_t) (ival) >= 0);                         \
        artx_NAME_DECL(task)                                               \
        static uint8_t task ## _stack[stack_size + artx_STACK_OVERHEAD];   \
        static struct artx_tcb task = {                                    \
          artx_MONITOR_TASK_INIT_(mon, task)                               \
          artx_SP_CXT_INIT_(task)                                          \
          .interval = ival,                                                \
          .priority = prio,                                                \
          .schedule = offset,                                              \
          .sp = (uint16_t) &task ## _stack[stack_size                      \
                                             + artx_STACK_OVERHEAD - 1]    \
        }

/**
 *  Allocate User Task with Scheduling Offset
 *
 *  \hideinitializer
 *
 *  This macro will allocate all resources required for a new
 *  user task. It will preinitialize the task's TCB and allocate
 *  the task's stack. But don't forget to call ARTX_task_init()
 *  to fully initialize the task.
 *
 *  There can only be one task per priority, so don't try to set
 *  up multiple tasks running at the same priority. If you want
 *  multiple routines to run at the same priority, consider
 *  using #ARTX_USE_MULTI_ROUT.
 *
 *  \param task                  The unique name of the task.
 *
 *  \param prio                  The unique user priority of the task.
 *
 *  \param ival                  The scheduling interval in multiples
 *                               of the tick interval.
 *
 *  \param stack_size            The user stack size in bytes. The stack
 *                               overhead required by the kernel will be
 *                               added automatically.
 *
 *  \param offset                The scheduling offset in multiples
 *                               of the tick interval.
 *
 *  Using the scheduling offset allows you to schedule different tasks
 *  at different times. For example, two tasks with the same interval
 *  could be running at a certain offset relative to each other so they
 *  wouldn't interfere with each other. It is also possible to just use
 *  the offset to define a startup delay for a task.
 */
#define ARTX_TASK_OFFS(task, prio, ival, stack_size, offset)               \
          ARTX_STATIC_ASSERT((int16_t) (ival) > 0);                        \
          ARTX_STATIC_ASSERT((prio) >= 0 && (prio) <= ARTX_PRIO_USER_MAX); \
          artx_ALLOC_TASK(task, (prio) + artx_PRIO_USER_OFFSET, ival,      \
                          stack_size, offset + 1)

/**
 *  Allocate User Task
 *
 *  \hideinitializer
 *
 *  This macro will allocate all resources required for a new
 *  user task. It will preinitialize the task's TCB and allocate
 *  the task's stack. But don't forget to call ARTX_task_init()
 *  to fully initialize the task.
 *
 *  There can only be one task per priority, so don't try to set
 *  up multiple tasks running at the same priority. If you want
 *  multiple routines to run at the same priority, consider
 *  using #ARTX_USE_MULTI_ROUT.
 *
 *  \param task                  The unique name of the task.
 *
 *  \param prio                  The unique user priority of the task.
 *
 *  \param ival                  The scheduling interval in multiples
 *                               of the tick interval.
 *
 *  \param stack_size            The user stack size in bytes. The stack
 *                               overhead required by the kernel will be
 *                               added automatically.
 */
#define ARTX_TASK(task, prio, ival, stack_size)                            \
          ARTX_TASK_OFFS(task, prio, ival, stack_size, 0)

/**
 *  Allocate Idle Task
 *
 *  \hideinitializer
 *
 *  This macro will allocate all resources required for the idle
 *  task. It will preinitialize the task's TCB and allocate
 *  the task's stack. But don't forget to call ARTX_task_init()
 *  to fully initialize the task.
 *
 *  There can only be one idle task, so don't try to set up
 *  multiple idle tasks. If you want multiple routines to run
 *  in the idle task, consider using #ARTX_USE_MULTI_ROUT.
 *
 *  \param task                  The unique name of the task.
 *
 *  \param stack_size            The user stack size in bytes. The stack
 *                               overhead required by the kernel will be
 *                               added automatically.
 */
#define ARTX_IDLE_TASK(task, stack_size)                                   \
          artx_ALLOC_TASK(task, artx_PRIO_IDLE, 0, stack_size, 0)

/**
 *  Allocate Routine
 *
 *  \hideinitializer
 *
 *  This macro will allocate all resources required for a routine.
 *  It will initialize the routine's RCB. Use ARTX_task_push_rout() to
 *  add the routine to a user task.
 *
 *  \param routine               The unique name of the routine.
 *
 *  An RCB will only be allocated for the routine if #ARTX_USE_MULTI_ROUT
 *  is defined to 1. If #ARTX_USE_ROUT_STATE is defined to 1, the routine
 *  will initially be disabled. You will then have to use ARTX_rout_enable()
 *  to enable the routine.
 */

#if ARTX_USE_MULTI_ROUT

#define ARTX_ROUT(routine)                                                 \
        artx_NAME_DECL(routine)                                            \
        static void routine ## _fun(void);                                 \
        static struct artx_rcb routine = {                                 \
          artx_MONITOR_ROUT_INIT_(mon, routine)                            \
          artx_INITIAL_ROUT_STATE_                                         \
          .rout = &routine ## _fun                                         \
        };                                                                 \
        static void routine ## _fun(void)

#else /* !ARTX_USE_MULTI_ROUT */

#define ARTX_ROUT(routine)                                                 \
        static void routine(void)

#endif /* ARTX_USE_MULTI_ROUT */

void ARTX_task_init(struct artx_tcb *tcb);

artxNAKED void ARTX_schedule(void);

/* TODO: Locking should be done by disabling the timer interrupt only.
 *       Agreed. Interrupts are safe to run on usertask stacks, as
 *       they can use the context stack, which should be enough.
 *       During interrupts, the global interrupt flag is cleared, so
 *       there are no nesting interrupts. Only drawback is that the
 *       interrupts will potentially disturb monitoring of the usertask
 *       stacks.
 */

#if ARTX_USE_MULTI_ROUT

void ARTX_task_push_rout(struct artx_tcb *tcb, struct artx_rcb *rout);

#else

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
 *  \param rout                  Pointer to the routine.
 */

static inline void ARTX_task_push_rout(struct artx_tcb *tcb, void (*rout)(void))
{
  tcb->rout = rout;
}

#endif // ARTX_USE_MULTI_ROUT

static inline void ARTX_task_set_interval(struct artx_tcb *tcb, uint16_t interval)
{
  tcb->interval = interval;
}

#if ARTX_USE_ROUT_STATE

/**
 *  Enable a routine
 *
 *  Enable a routine that is enqueued in a tasks run queue.
 *
 *  \param rout                  Pointer to the routine control block.
 */

static inline void ARTX_rout_enable(struct artx_rcb *rout)
{
  rout->state = ARTX_RS_ENABLED;
}

/**
 *  Disable a routine
 *
 *  Disable a routine that is enqueued in a tasks run queue.
 *
 *  \param rout                  Pointer to the routine control block.
 */

static inline void ARTX_rout_disable(struct artx_rcb *rout)
{
  rout->state = ARTX_RS_DISABLED;
}

/**
 *  Get the state of a routine
 *
 *  This routine returns the current state of a routine.
 *
 *  \param rout                  Pointer to the routine control block.
 */

static inline enum ARTX_rout_state ARTX_rout_get_state(const struct artx_rcb *rout)
{
  return rout->state;
}

#endif // ARTX_USE_ROUT_STATE

#if ARTX_ENABLE_TICK_SYNC

void ARTX_tick_sync(void);

void ARTX_get_sync_status(struct ARTX_sync_status *status);

#endif

#if ARTX_ENABLE_TIME

void ARTX_hires_time(struct ARTX_timeval *p_time);

void ARTX_delta_time(const struct ARTX_timeval *p_t0, const struct ARTX_timeval *p_t1, struct ARTX_timeval *p_delta);

uint32_t ARTX_time(void);

uint32_t ARTX_us_time(void);

#endif

#endif
