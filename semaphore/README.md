# Understanding Semaphores

A queue moves *data* between tasks. A semaphore moves *permission*: it is a
signal, or a key, passed between tasks, or between an interrupt and a task, and
it carries no data at all. That single idea covers three closely related objects
that overlap heavily but differ in important details: binary semaphores, counting
semaphores, and mutexes. The runnable examples live under [`app/Src/`](app/Src/),
one file per concept, and are built on the project skeleton described under
[Template](../template/README.md).

Everything here is already enabled in this module's `FreeRTOSConfig.h`
(`configUSE_MUTEXES` and `configUSE_COUNTING_SEMAPHORES` are both `1`), and every
example needs `#include "semphr.h"`.

## Binary Semaphores

A binary semaphore holds one of two values, taken or available: you either have
the key or you do not. It is created with `xSemaphoreCreateBinary`, and the one
detail that catches everybody is that it is created **empty**. A task that tries
to take it before anyone has given it will block:

```c
SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary();

xSemaphoreTake( xSemaphore, portMAX_DELAY );  /* wait for the key */
xSemaphoreGive( xSemaphore );                 /* hand the key back */
```

`xSemaphoreTake` takes the handle and a block time, exactly like a queue read,
and `xSemaphoreGive` returns the key so somebody else can have it. Because a
binary semaphore is a single flag, giving it twice before it is taken has the
same effect as giving it once; the second give is simply lost. That is the
behavior counting semaphores exist to fix. See `sem_binary.c`.

## Semaphores from an Interrupt

The most valuable use of a binary semaphore is *deferred interrupt handling*: the
ISR does almost nothing except signal a task, and the real work happens in that
task, at task priority, where it is allowed to block. FreeRTOS marks every API
that is legal inside an ISR with a `FromISR` suffix, and only those may be called
from an interrupt; `xSemaphoreGive` is forbidden there, `xSemaphoreGiveFromISR`
is the one to use.

Its second argument is what makes the pattern work. `xHigherPriorityTaskWoken` is
set to `pdTRUE` if the give unblocked a task more important than the one that was
interrupted, and passing it to `portYIELD_FROM_ISR` makes the switch to that task
happen as the interrupt returns, instead of waiting for the next tick:

```c
void EXTI0_IRQHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    EXTI->PR |= ( 1 << 0 );   /* clear the pending bit */

    xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```

There is a hardware trap here worth knowing about. FreeRTOS only tolerates
`FromISR` calls from interrupts whose priority is numerically greater than or
equal to `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`, which is `5` in this
project. After reset every interrupt sits at priority `0`, the highest, so an
interrupt that has not had its priority lowered by hand will trip `configASSERT`
and hang the moment it calls into the kernel. See `sem_binary_isr.c`.

## Counting Semaphores

A counting semaphore carries a counter instead of a flag. The count is the number
of keys currently available, so it is the right tool when there are several
equivalent instances of something, or when events arrive faster than they can be
processed and none of them may be dropped. Each give increments the count, each
take decrements it, and a task blocks only when the count is zero:

```c
/* max count, initial count */
SemaphoreHandle_t xSemaphore = xSemaphoreCreateCounting( 10, 0 );
```

The two arguments are the maximum count and the initial count. For counting
events you start at `0` and let the givers push it up; for managing a pool of
resources you start the count at the size of the pool, so the first takers
succeed immediately and the task that arrives when the pool is empty waits. See
`sem_counting.c`.

## Mutexes

A mutex, short for *mutual exclusion*, is a binary semaphore with a different
purpose and one extra behavior. It exists to let several tasks share one resource
while guaranteeing that only one of them touches it at a time. It is created
available, so the first task to take it succeeds, and it is used to bracket the
critical section:

```c
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

xSemaphoreTake( xMutex, portMAX_DELAY );
printf( "only one task at a time gets here\r\n" );
xSemaphoreGive( xMutex );
```

The rule that distinguishes a mutex from a binary semaphore is ownership: the
task that takes a mutex is the one that must give it back. A binary semaphore has
no owner and can legitimately be given by a task or interrupt that never took it,
which is why signalling uses a semaphore and locking uses a mutex. See
`sem_mutex.c`.