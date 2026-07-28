# Understanding Event Groups

A semaphore signals one event; a queue carries data. An *event group* signals a
*combination* of events. It lets a task wait in the Blocked state until one, or
several, of many independent things have happened, and it lets several tasks
react to the same event at once. Because a single event group can stand in for a
whole handful of binary semaphores, it also saves RAM. The runnable examples live
under [`app/Src/`](app/Src/), one file per concept, and are built on the project
skeleton described under [Template](../template/README.md).

Event groups are enabled by default (`configUSE_EVENT_GROUPS`), and every example
needs `#include "event_groups.h"`.

## Event Bits

An event group is nothing more than a set of bits held in one variable. Each bit
is an *event flag*: a boolean that is `1` when its event has happened and `0` when
it has not. How many bits you get depends on the tick width: with
`configUSE_16_BIT_TICKS` set to `1` a group holds 8 usable bits, and with it set
to `0`, as in this project, a group holds **24 usable bits**, numbered 0 to 23.
You give the bits meaning yourself, usually with named masks:

```c
#define BIT_A   ( 1 << 0 )
#define BIT_B   ( 1 << 1 )
#define BIT_C   ( 1 << 2 )
```

A group is created with `xEventGroupCreate`, which returns a handle or `NULL`, and
starts with every bit clear:

```c
EventGroupHandle_t xEventGroup = xEventGroupCreate();
```

## Setting, Clearing, and Reading Bits

Three calls cover the basics. `xEventGroupSetBits` turns one or more bits on,
`xEventGroupClearBits` turns them off, and `xEventGroupGetBits` returns the whole
mask so you can inspect it without blocking:

```c
xEventGroupSetBits( xEventGroup, BIT_A );              /* turn BIT_A on  */
xEventGroupClearBits( xEventGroup, BIT_A | BIT_B );    /* turn two off   */
EventBits_t uxBits = xEventGroupGetBits( xEventGroup ); /* read the mask */
```

Setting an already-set bit is harmless, and any number of tasks may set bits into
the same group; the kernel keeps the updates consistent. See `eg_basics.c`, which
turns three bits on and off while another task mirrors them onto the LEDs.

## Waiting on a Combination

The real power is blocking until a combination of bits appears, with
`xEventGroupWaitBits`. It takes the group, the bits to wait for, and two flags
that trip people up:

```c
EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToWaitFor,
                                 const BaseType_t xClearOnExit,
                                 const BaseType_t xWaitForAllBits,
                                 TickType_t xTicksToWait );
```

`xWaitForAllBits` chooses the logic: `pdTRUE` unblocks only when **all** of the
requested bits are set (AND), while `pdFALSE` unblocks as soon as **any** one of
them is set (OR). `xClearOnExit` decides cleanup: `pdTRUE` clears the satisfying
bits before the call returns, so the group re-arms itself for the next round;
`pdFALSE` leaves them set. Either way the return value is the bit mask at the
moment the task woke, so after an OR wait you can test the result to see which
event actually fired. `eg_wait_all.c` shows the AND case with three tasks setting
bits into one group, and `eg_wait_any.c` shows the OR case waking on either of two
sources.

## Synchronizing Tasks

Sometimes tasks must meet at a point where none may go on until all have arrived,
a *rendezvous* or barrier. `xEventGroupSync` does exactly this in one atomic step:
it sets the calling task's own bit and then blocks until a whole set of bits is
present.

```c
EventBits_t xEventGroupSync( EventGroupHandle_t xEventGroup,
                             const EventBits_t uxBitsToSet,
                             const EventBits_t uxBitsToWaitFor,
                             TickType_t xTicksToWait );
```

Each participant passes its own bit as `uxBitsToSet` and the combined mask of all
participants as `uxBitsToWaitFor`. As each task reaches the sync point it announces
itself and waits; when the last one arrives, every task is released together and
the sync bits are cleared automatically, ready for the next round. See
`eg_sync.c`, where three tasks arrive at staggered times but proceed in lockstep.

## Event Bits from an Interrupt

An interrupt can set event bits too, with `xEventGroupSetBitsFromISR`, which turns
a group into a clean way to defer interrupt work to a task. There is a twist worth
knowing: setting bits may unblock several waiting tasks, which is unbounded work
that an ISR must not do, so the kernel *defers* the actual set to the timer daemon
task. That is why event groups depend on `configUSE_TIMERS` being enabled, and why
the call returns `pdPASS` or `pdFAIL` depending on whether the request reached the
daemon's queue. The usual interrupt rule still applies: the interrupt must be
given an NVIC priority numerically at or above
`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` (5 here), set before it is enabled.
See `eg_isr.c`.

## Apps

Each app is a self-contained `main` that demonstrates one concept, watched on the
board LEDs (red, yellow, green), on the serial port (UART2, 115200 8N1), or driven
by the button on `PA0`.

1. [Setting and Reading Bits](app/Src/eg_basics.c): one task sets three bits on and
   off in turn while another reads the mask and mirrors each bit onto an LED.
2. [Multiple Setters, One Waiter](app/Src/eg_wait_all.c): three tasks set their own
   bit into a shared group and a waiter fires only when all three are present,
   then re-arms with `xClearOnExit`.
3. [Waiting for Any Bit](app/Src/eg_wait_any.c): a waiter blocks on two sources with
   `xWaitForAllBits` false, waking on either a periodic tick or a button press and
   reading the result to tell which.
4. [Synchronizing Tasks](app/Src/eg_sync.c): three tasks meet at a barrier with
   `xEventGroupSync`, arriving at different times but passing together.
5. [Bits from an ISR](app/Src/eg_isr.c): the button's EXTI interrupt sets a bit with
   `xEventGroupSetBitsFromISR`, and a task blocked on that bit does the deferred
   work.