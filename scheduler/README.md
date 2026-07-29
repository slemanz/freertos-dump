# Understanding the Scheduler

The scheduler is the routine at the heart of the kernel that decides which
Ready task moves into the Running state, and when. Everything else in FreeRTOS,
tasks, priorities, blocking, exists so the scheduler can make that decision
predictably. This chapter looks at the rules it follows and, crucially, at the
few configuration switches that change them, because the scheduler's behavior
*is* its configuration. The runnable examples live under [`app/Src/`](app/Src/),
one file per concept, and are built on the project skeleton described under
[Template](../template/README.md).

Unlike the other chapters, several examples here require editing
`FreeRTOSConfig.h` and rebuilding. This project ships with
`configUSE_PREEMPTION` set to `1` and no `configUSE_TIME_SLICING` line, so it
defaults to `1`: prioritized, preemptive, time-sliced. The tick runs at 1000 Hz,
so a time slice is one millisecond.

## Fixed Priority

FreeRTOS uses *fixed-priority* scheduling: the scheduler never changes the
priority you assign to a task. It always runs the highest-priority task that is
Ready, and it will not quietly demote or promote anyone to be fair. "Fixed" here
means the scheduler leaves priorities alone, not that they can never change, your
code is still free to change a task's priority with `vTaskPrioritySet`, and a
task may change its own or another's. The scheduler simply respects whatever the
priorities currently are.

## Preemption

With `configUSE_PREEMPTION` set to `1`, the scheduler is *preemptive*: the moment
a task of higher priority than the running one becomes Ready, it takes the CPU at
once. "Preempt" means the running task is pushed out of the Running state back to
Ready without asking, mid-execution, so the more important task can run. This is
what makes the system responsive: a high-priority task waiting on an event runs
almost immediately after that event, rather than at the mercy of whatever is
currently executing. Preemption is driven entirely by priority, so it says
nothing about tasks that share the same priority; that is the next question.

## Round-Robin and Time Slicing

When several Ready tasks share the highest priority, the scheduler runs them
*round-robin*, one after another. It makes no promise that they get exactly equal
time, only that each equal-priority Ready task gets its turn. What decides how
often turns rotate is `configUSE_TIME_SLICING`.

With time slicing on, the scheduler switches to the next equal-priority task at
the end of every tick, even if the running task never blocks or yields. Three
equal-priority tasks with a 1 ms slice each run for roughly a millisecond in
turn. With time slicing off, that automatic rotation is gone: a running task
keeps the CPU until it blocks, yields, or is preempted by something of higher
priority, so among equal-priority CPU-bound tasks the first one to run
monopolizes the processor and the others starve. `sched_preempt_only.c`
(time slicing off) and `sched_timeslicing.c` (time slicing on) are the same
program under the two settings, and the difference is stark: one LED moving
versus all three.

## Cooperative Scheduling

Setting `configUSE_PREEMPTION` to `0` selects the *cooperative* scheduler, the
opposite extreme. Now the scheduler never forces a switch at all. A task runs
until it voluntarily gives up the CPU, either by blocking on a kernel call such
as `vTaskDelay`, or by calling `taskYIELD()` to offer the processor to another
Ready task of equal or higher priority. Even a higher-priority task that becomes
Ready has to wait until the running task lets go. Cooperative scheduling is
simple and has no preemption races, but it is only as responsive as your tasks
are disciplined about yielding. See `sched_cooperative.c`, where two tasks take
turns solely because each one yields.

## Suspending the Scheduler

Sometimes a task needs to run a short section with no danger of being switched
out, without going as far as disabling interrupts. `vTaskSuspendAll` locks the
scheduler and `xTaskResumeAll` unlocks it; between the two, no context switch to
another task can happen, so the section is atomic with respect to other tasks.
The important distinction from a critical section (`taskENTER_CRITICAL`) is that
suspending the scheduler leaves interrupts fully enabled, ISRs still fire, it only
stops task switching. The one rule is that you must not call any blocking API
while the scheduler is suspended. See `sched_suspend.c`, where each task's
multi-line output is kept whole.

## Apps

Each app is a self-contained `main` that demonstrates one concept, watched on the
board LEDs (red, yellow, green) and on the serial port (UART2, 115200 8N1). Build
each one with its own make target (see the table at the top of `temp.md`), which
passes the right scheduler configuration as a `-D` define; there is no
`FreeRTOSConfig.h` to edit by hand.

1. [Preempt Only](app/Src/sched_preempt_only.c): three equal-priority workers with
   `configUSE_TIME_SLICING` at 0, so one monopolizes the CPU and the other two
   starve.
2. [Pseudo Time Slicing](app/Src/sched_timeslicing.c): the same program with time
   slicing at 1, so the three workers share the CPU round-robin and all three
   counters climb together.
3. [Cooperative](app/Src/sched_cooperative.c): with `configUSE_PREEMPTION` at 0,
   two tasks take turns only because each calls `taskYIELD`; remove a yield and
   that task hogs the CPU forever.
4. [Suspending the Scheduler](app/Src/sched_suspend.c): `vTaskSuspendAll` and
   `xTaskResumeAll` keep each task's block of output whole without disabling
   interrupts.