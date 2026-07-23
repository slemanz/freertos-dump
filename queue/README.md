# Queue and Queueset Management

Tasks are deliberately isolated from one another, each with its own stack, so
they need a safe channel to exchange data. That channel is the *queue*: the
primary mechanism FreeRTOS gives you for passing information between tasks (and
between interrupts and tasks) without sharing raw variables and the race
conditions that come with them. The runnable examples live under
[`app/Src/`](app/Src/), one file per concept, and are built on the project
skeleton described under [Template](../template/README.md).

## What a Queue Is

A queue is a FIFO buffer: data is inserted at the **back** and removed from the
**front**. It holds a finite number of fixed-size items, and the maximum number
it can hold is the *length* of the queue. Both the length and the size of a
single item are fixed when the queue is created and never change. A queue is
owned by no task in particular; any task that has its handle can write to it or
read from it.

## Passing Data: by Value or by Reference

There are two ways to move data through a queue. **By value** copies the item
itself into the queue and back out again, which is simple and keeps the data
self-contained. **By reference** puts a *pointer* to the data into the queue
instead of the data itself. Passing by reference is the preferred approach for
large items, because copying a pointer is cheap no matter how big the payload
is, whereas copying the whole item byte by byte is not. The catch is that the
pointed-to data must stay valid until the receiver is done with it.

## Blocking on a Queue

Reading and writing can both block, which is what makes a queue useful for
coordination and not just transport. When a task reads from an empty queue it
can specify a *block time*: it is put into the Blocked state, using no CPU, until
either data arrives or the timeout expires, and it returns to Ready the moment an
item becomes available. Writing is symmetric: a task that writes to a full queue
is blocked until space opens up. A block time of `0` returns immediately, and
`portMAX_DELAY` waits forever.

## Creating and Using a Queue

A queue is created with `xQueueCreate`, giving the length and the size of one
item; it returns a handle, or `NULL` if there was not enough heap:

```c
QueueHandle_t xQueue = xQueueCreate( 5, sizeof(uint32_t) );  /* 5 items */
```

`xQueueSend` (identical to `xQueueSendToBack`) writes one item at the back, and
`xQueueReceive` reads one item from the front. Both take the queue handle, a
pointer to the buffer to copy from or into, and a block time:

```c
uint32_t ulValue = 42;
xQueueSend( xQueue, &ulValue, portMAX_DELAY );

uint32_t ulReceived;
if( xQueueReceive( xQueue, &ulReceived, portMAX_DELAY ) == pdPASS )
{
    /* ulReceived now holds a copy of what the sender wrote. */
}
```

Because the receiver can wait on the queue with no timer of its own, the queue
also works as a synchronization primitive: the receiver's pace is driven
entirely by the sender. See `queue_sync.c`.

## Sending More Complex Data

An item does not have to be a scalar. As long as every item is the same fixed
size, it can be a struct, which lets one message carry several fields at once,
such as a command plus its argument, or an identifier telling the receiver which
sender it came from:

```c
typedef struct
{
    uint8_t  id;
    uint32_t value;
} Message_t;

QueueHandle_t xQueue = xQueueCreate( 5, sizeof(Message_t) );
```

Sending the struct by value copies the whole thing through the queue. For a
large payload you would instead size the queue for a *pointer* and send the
address, saving both memory and copy time. See `queue_struct.c`.

## Queue Sets

Sometimes a task needs data from more than one queue and cannot know in advance
which one will have it first. Polling each queue in turn is wasteful; a **queue
set** solves it by letting a task block on several queues at once and be told
which one became ready. A queue set can also hold semaphores, not just queues.

Enable it with `configUSE_QUEUE_SETS` set to `1` in `FreeRTOSConfig.h`, then
build the set with three calls. `xQueueCreateSet` creates the set, sized to hold
the combined length of every queue that will join it. `xQueueAddToSet` adds a
queue (or semaphore) to the set, and must be done while that queue is empty.
`xQueueSelectFromSet` blocks until any member has data and returns the handle of
the one that did, which you then read from as usual:

```c
QueueSetMemberHandle_t xActivated = xQueueSelectFromSet( xQueueSet, portMAX_DELAY );

if( xActivated == xRedQueue )
{
    xQueueReceive( xRedQueue, &ulReceived, 0 );   /* guaranteed to succeed */
}
```

See `queue_set.c`.

## Apps

Each app is a self-contained `main` that demonstrates one concept, and most of
them are meant to be watched on the board LEDs (red, yellow, green) or read on
the serial port (UART2, 115200 8N1).

1. [Working with Queues](app/Src/queue_basics.c): a sender pushes an
   incrementing counter into a queue once per second and a receiver blocks on
   the queue, toggling an LED each time an item arrives.
2. [Updating printf](app/Src/queue_printf.c): the same producer/consumer, but a
   single printer task owns the serial port and prints every value it takes off
   the queue, so log output is never interleaved.
3. [Synchronizing Tasks with Queues](app/Src/queue_sync.c): a consumer with no
   delay of its own runs only when a producer signals it through a queue, so the
   two LEDs move in lockstep and the queue acts as the synchronization.
4. [Sending More Complex Data](app/Src/queue_struct.c): two senders reuse one
   task function to push a struct carrying their own id into a shared queue, and
   the receiver toggles a different LED depending on which sender it came from.
5. [Working with Queue Sets](app/Src/queue_set.c): two senders write to two
   separate queues at different rates while one receiver waits on a queue set and
   services whichever queue delivered. Needs `configUSE_QUEUE_SETS` set to 1.
