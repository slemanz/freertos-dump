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