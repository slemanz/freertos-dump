# Thread Management

A FreeRTOS application is structured as a set of tasks, and almost everything you
do begins with creating one. This chapter covers how a task comes into being, how
the scheduler takes over, and how tasks can spawn further tasks at run time. The
runnable examples live under [`app/Src/`](app/Src/), one file per concept, and are
built on the project skeleton described under [Template](../template/README.md).

## Creating a Task

A task is created with `xTaskCreate`, which allocates the task's stack and control
block and registers it with the scheduler:

```c
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
                        const char *const pcName,
                        uint16_t usStackDepth,
                        void *pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t *pxCreatedTask );
```

Each parameter has a clear role. `pvTaskCode` is the function that implements the
task, an infinite loop that never returns. `pcName` is a human-readable label
used only for debugging. `usStackDepth` sizes the task's private stack, counted
in words rather than bytes. `pvParameters` is an arbitrary pointer passed
straight through to the task, which makes it easy to reuse one function across
several tasks. `uxPriority` sets the scheduling priority, with higher values
winning the CPU. Finally, `pxCreatedTask` can receive a handle to the new task,
or `NULL` when you never need to refer to it again.

The task function itself follows a fixed shape: configure whatever it owns, then
loop forever, yielding the CPU with a blocking call such as `vTaskDelay` so that
lower-priority tasks get their turn.

```c
void vTaskLED(void *pvParameters)
{
    /* one-time setup ... */
    while (1)
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

## Starting the Scheduler

Creating tasks does not run them. In `main`, the tasks are registered and then
control is handed to the kernel with `vTaskStartScheduler()`, which selects the
highest-priority ready task and never returns:

```c
int main(void)
{
    config_app();

    xTaskCreate(vTaskLED, "LED", 1000, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) { }   /* only reached if the scheduler runs out of heap */
}
```

## Creating Tasks from Other Tasks

`xTaskCreate` is not restricted to `main`; a task that is already running can
create more tasks just as easily. Because the scheduler is live, the new tasks
become eligible to run immediately, and you must *not* call
`vTaskStartScheduler()` a second time.

```c
void vBlueLedControllerTask(void *pvParameters)
{
    xTaskCreate(vRedLedControllerTask,   "Red Led Controller",   100, NULL, 1, NULL);
    xTaskCreate(vGreenLedControllerTask, "Green Led Controller", 100, NULL, 1, NULL);

    while (1)
    {
        BlueTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

This pattern is common when one task acts as a supervisor that brings the others
online only once some precondition, a peripheral finishing its startup, say, has
been met.

## Task States

At any moment a task sits in exactly one of four states, and understanding them
is the key to reasoning about everything else. A **Ready** task can run but is
waiting for its turn. The **Running** task is the one currently holding the CPU,
and on a single core there is only ever one. A **Blocked** task is waiting for
something, either a timeout or a synchronization event such as a semaphore, and
it burns no CPU while it waits. A **Suspended** task is taken out of scheduling
entirely until it is explicitly resumed. The scheduler moves tasks between Ready
and Running on its own; every other transition happens because your code called a
blocking or a control API.

## Priorities and Preemption

Every task is given a priority, where a higher number means higher priority, up to
`configMAX_PRIORITIES`. The scheduler always runs the highest-priority task that is
ready. With preemption enabled (`configUSE_PREEMPTION`), a task that becomes ready
at a higher priority than the running one takes over the CPU right away, without
waiting for the current task to cooperate. Tasks that share the same priority take
turns in round-robin fashion. The trap to remember: a high-priority task that never
blocks will starve everything below it, so tasks are expected to block regularly.
See `task_priority.c`.

## Changing Priority at Runtime

Priorities are not fixed at creation. `uxTaskPriorityGet()` reads a task's current
priority and `vTaskPrioritySet()` changes it while the system runs. Both act on a
task handle, so you need to keep the handle returned by `xTaskCreate` for any task
you intend to adjust from the outside. See `task_priority_runtime.c`.

## Suspending, Resuming, and Deleting

`vTaskSuspend()` pulls a task out of scheduling no matter its priority, and it stays
out until `vTaskResume()` puts it back into the Ready state. Suspension is not
counted, so several suspend calls are undone by a single resume. `vTaskDelete()`
removes a task for good; passing `NULL` deletes the calling task itself and never
returns. The memory of a deleted task is reclaimed by the Idle task, so the Idle
task must be given some CPU time. In all cases a task function must loop forever or
delete itself, never fall off its end.

## Blocking and Delays

The Blocked state is how a task waits without wasting cycles. `vTaskDelay()` blocks
the caller for a number of ticks and then returns it to Ready, and the
`pdMS_TO_TICKS()` macro converts milliseconds into ticks so you can think in real
time. This is the opposite of a busy-wait loop, which keeps the task Ready and
spins the CPU for the whole wait, starving lower-priority work. Reaching for a
blocking delay instead of a busy loop is the single most important habit in RTOS
code. See `task_blocking.c`.

## Periodic Execution

`vTaskDelay()` counts its delay from the moment it is called, so the real period
becomes the work time plus the delay and drifts whenever the work takes longer.
For a fixed cadence, `xTaskDelayUntil()` wakes the task at an absolute interval that
does not drift. You initialize a reference tick once before the loop and the call
updates it for you on every pass:

```c
TickType_t xLastWakeTime = xTaskGetTickCount();
for (;;)
{
    do_work();
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
}
```

This is what you want for sampling and control loops. See `task_periodic.c`.

## The Idle Task

The scheduler needs at least one runnable task at all times, so it creates the Idle
task automatically at the lowest priority when you call `vTaskStartScheduler()`. It
runs only when nothing else is ready, and it is where the memory of deleted tasks is
freed. Setting `configUSE_IDLE_HOOK` to 1 makes the kernel call
`vApplicationIdleHook()` once per idle loop, which is a convenient place for low
priority background work. The hook must never block and must return quickly.
`configIDLE_SHOULD_YIELD` controls whether the Idle task gives up the rest of its
slice as soon as an equal-priority task wakes up. See `task_idle_hook.c`.

## The Tick Hook

Setting `configUSE_TICK_HOOK` to 1 makes the kernel call `vApplicationTickHook()`
on every tick interrupt. Because it runs inside the interrupt, it has to be tiny,
and it may only call APIs whose names end in `FromISR`. In practice it adds jitter
and works against the real-time behavior you want, so keep it minimal or avoid it
unless you have a clear reason.

## Apps

Each app is a self-contained `main` that demonstrates one concept, and most of
them are meant to be watched on the three board LEDs (red, yellow, green).

1. [Task Creation](app/Src/task_creation.c): three independent tasks at the same
   priority, each bumping its own counter so you can confirm in the debugger that
   all of them run.
2. [Tasks Leds](app/Src/tasks_leds.c): one task per LED, each blinking at a
   different rate, so the three LEDs blink independently.
3. [Task Parameters](app/Src/task_parameters.c): a single task function reused for
   three LEDs by passing a different pin as the task parameter.
4. [Task Priority](app/Src/task_priority.c): a high-priority task preempts a
   low-priority one and only lets it run while blocked, so the low LED moves only
   in the gaps.
5. [Task Priority Runtime](app/Src/task_priority_runtime.c): a controller task
   reads and flips a worker task's priority on the fly with `vTaskPrioritySet`.
6. [Task Suspend](app/Src/task_suspend.c): after a few seconds one task's LED
   freezes for good while another keeps blinking, proving the scheduler is fine.
7. [Task Resume](app/Src/task_resume.c): a task is suspended and resumed in a
   cycle, so its LED alternates between blinking and frozen.
8. [Task Delete](app/Src/task_delete.c): one task deletes itself and another is
   deleted by a peer, both LEDs going dark for good while a third keeps running.
9. [Task Blocking](app/Src/task_blocking.c): a busy-wait task starves a
   lower-priority one; swap the busy loop for `vTaskDelay` and the starved LED
   comes back to life.
10. [Task Periodic](app/Src/task_periodic.c): `vTaskDelay` drifts against a fixed
    period while `xTaskDelayUntil` holds it, so one LED slowly slips behind the
    other.
11. [Task Idle Hook](app/Src/task_idle_hook.c): the idle hook blinks an LED so
    fast it looks dim, and it freezes whenever a task hogs the CPU. Needs
    `configUSE_IDLE_HOOK` set to 1.
