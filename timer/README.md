# Software Timers

A software timer runs a function at a chosen point in the future, either once
or over and over at a fixed period. The function it runs is called the timer's
*callback*. These timers are a pure software service of the kernel: they are not
the hardware timer peripherals of the microcontroller, they need no such
peripheral, and any number of them can run at once. The runnable examples live
under [`app/Src/`](app/Src/), one file per concept, and are built on the project
skeleton described under [Template](../template/README.md).

Software timers are switched on with `configUSE_TIMERS`, which is already `1` in
this module's `FreeRTOSConfig.h`, alongside the three settings that size the
service described below. Every example needs `#include "timers.h"`.

## The Timer Service Task

There is a subtlety that shapes everything else: a software timer has no task of
its own. When a timer expires, its callback is run by a single kernel task, the
*timer service task*, also called the timer daemon, which the scheduler creates
for you when `configUSE_TIMERS` is set. That task is created automatically and
its properties come from the config file: `configTIMER_TASK_PRIORITY` sets its
priority, `configTIMER_TASK_STACK_DEPTH` sizes the one stack that every callback
shares, and `configTIMER_QUEUE_LENGTH` sizes the queue that API calls such as
`xTimerStart` use to send commands to it.

Two rules follow directly from this. First, a callback must be short and must
**never block**: it may not call `vTaskDelay`, wait on a semaphore, or issue a
timer command with a non-zero block time from inside itself, because blocking the
daemon stalls every other timer. Second, all callbacks run at the daemon's
priority, so a callback does not preempt a higher-priority task; it is scheduled
like any other work.

## One-Shot and Auto-Reload Timers

Every timer is one of two kinds, chosen at creation. An **auto-reload** timer
restarts itself the moment it expires, so its callback runs periodically at a
fixed frequency forever. A **one-shot** timer runs its callback exactly once and
then stops; it does not restart itself, and it will only run again if the
application restarts it by hand. That single difference is the whole distinction,
and `timer_oneshot.c` shows the two side by side.

The behavior is easiest to picture as two states. A timer that is not counting is
**dormant**; a timer that is counting toward its next expiry is **running**. A
newly created timer is dormant. Starting, resetting, or changing the period of a
timer moves it to running. An auto-reload timer stays running across each expiry;
a one-shot timer returns to dormant when it expires. Either kind returns to
dormant when it is explicitly stopped.

## Creating and Starting a Timer

A timer is created with `xTimerCreate`, which takes five arguments and returns a
handle, or `NULL` if it could not be allocated:

```c
TimerHandle_t xTimerCreate( const char *const pcTimerName,
                            TickType_t xTimerPeriodInTicks,
                            BaseType_t xAutoReload,
                            void *pvTimerID,
                            TimerCallbackFunction_t pxCallbackFunction );
```

`pcTimerName` is a debug-only label. `xTimerPeriodInTicks` is the period, which
must be greater than zero and is most readable through `pdMS_TO_TICKS()`.
`xAutoReload` picks the kind: `pdTRUE` for auto-reload, `pdFALSE` for one-shot.
`pvTimerID` is an arbitrary pointer stored with the timer, discussed below.
`pxCallbackFunction` is the function to run on expiry.

Creation only makes the timer; it starts dormant. `xTimerStart` moves it to the
running state. Because starting is really a command posted to the daemon's queue,
you can call it in `main` before the scheduler runs, and it simply takes effect
once the scheduler starts:

```c
xBlinkTimer = xTimerCreate( "Blink", pdMS_TO_TICKS( 500 ), pdTRUE, NULL, vBlinkCallback );

if( xBlinkTimer != NULL )
{
    xTimerStart( xBlinkTimer, 0 );
    vTaskStartScheduler();
}
```

See `timer_basics.c`, which blinks an LED entirely from the callback, with no
user task at all.

## The Callback Function

Every callback has the same fixed prototype: it returns nothing and takes the
handle of the timer that expired.

```c
void vBlinkCallback( TimerHandle_t xTimer )
{
    GPIO_ToggleOutputPin( GPIOB, LED_RED );
}
```

Receiving the handle is what lets one function serve several timers, and it is
the hook for the timer ID described below. Keep the body small, and remember it
runs in the daemon, not in the task that started the timer.