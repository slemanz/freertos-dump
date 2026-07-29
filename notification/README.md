# Understanding Task Notifications

Every synchronization tool so far, semaphores, queues, event groups, works through
a separate kernel object that both sides refer to. A *task notification* removes
that middleman: it lets one task signal another, or send it a value, **directly**.
Each task is born with one notification built in, so nothing is allocated, and the
result is the fastest and most memory-efficient way for two tasks to communicate.
The runnable examples live under [`app/Src/`](app/Src/), one file per concept, and
are built on the project skeleton described under [Template](../template/README.md).

Notifications are enabled by `configUSE_TASK_NOTIFICATIONS`, which defaults to `1`,
and the API lives in `task.h` with no separate header.

## What a Notification Is

A notification is a per-task state with two parts: a **notification value**, a
single 32-bit word, and a **pending flag**. When a task is notified, its state
becomes *pending*; when the task reads the notification, the state returns to *not
pending*. Because the value is a full 32-bit word, a notification can act as a
simple signal, a small integer, a counter, or a set of bits, depending on how you
send and read it. That flexibility is why one primitive can replace so many
others.

The savings are real. To pass a signal through a semaphore, the kernel allocates a
semaphore; through a queue, it allocates a queue and copies data in and out. A
notification needs none of that, and updating it is a direct operation on the
target task, which is why it is both faster and lighter.

## Two Limitations

The speed comes with two rules that the other primitives do not have. First, a
notification is strictly **one-to-one**: it has a single sender at a time and
exactly one receiver, the task being notified. There is no one-to-many broadcast
like an event group can do. Second, it is **one-directional with respect to
interrupts**: a task or an ISR can notify a task, but a task can *not* notify an
ISR. Sending always needs the receiver's `TaskHandle_t`, which you keep from the
last argument of `xTaskCreate`.

## As a Binary Semaphore

The simplest use is a bare signal. `xTaskNotifyGive` pokes the target task, and
`ulTaskNotifyTake` blocks until the poke arrives:

```c
xTaskNotifyGive( xReceiverHandle );        /* in the sender */
ulTaskNotifyTake( pdTRUE, portMAX_DELAY ); /* in the receiver: wait, then clear */
```

The `pdTRUE` argument clears the value to zero on the way out, so it behaves as an
on/off flag, exactly like a binary semaphore. See `notify_basics.c`.

## As a Mailbox

Because the notification holds a value, the sender can deliver data instead of a
bare signal. `xTaskNotify` with the `eSetValueWithOverwrite` action writes a
32-bit word straight into the receiver, and `xTaskNotifyWait` reads it back:

```c
xTaskNotify( xReceiverHandle, ulValue, eSetValueWithOverwrite );
xTaskNotifyWait( 0, 0xFFFFFFFF, &ulReceived, portMAX_DELAY );
```

This is the "cheaper than a queue" case for a single writer and single reader. The
last mask, `0xFFFFFFFF`, clears the whole value on exit so each read starts fresh.
See `notify_value.c`.

## As a Counting Semaphore

Reading with `pdFALSE` instead of `pdTRUE` decrements the value by one rather than
clearing it, so counts are preserved. If several gives arrive before the receiver
runs, they accumulate and the receiver drains them one at a time, losing none of
them, which is precisely what a counting semaphore does:

```c
xTaskNotifyGive( xReceiverHandle );        /* each give is one event */
ulTaskNotifyTake( pdFALSE, portMAX_DELAY );/* takes one event, keeps the rest */
```

See `notify_counting.c`, where a burst of three events is handled one by one.

## As an Event Group

With the `eSetBits` action the notification value is treated as a set of flags:
each notify ORs bits in, and the receiver reads and tests them. It is a lightweight
event group, subject to the one-receiver rule.

```c
xTaskNotify( xReceiverHandle, BIT_ONE, eSetBits );
xTaskNotifyWait( 0, 0xFFFFFFFF, &ulBits, portMAX_DELAY );
```

See `notify_bits.c`, where two senders set different bits into one receiver.

## From an Interrupt

The one direction that works across the interrupt boundary is ISR to task, using
the `FromISR` twins. `vTaskNotifyGiveFromISR` is the give flavor, and like every
`FromISR` call it takes a `pxHigherPriorityTaskWoken` flag to hand to
`portYIELD_FROM_ISR`, and it may only be used from an interrupt whose NVIC priority
is numerically at or above `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` (5),
set before the interrupt is enabled. See `notify_isr.c`.

## Apps

Each app is a self-contained `main` that demonstrates one concept, watched on the
board LEDs (red, yellow, green), on the serial port (UART2, 115200 8N1), or driven
by the button on `PA0`.

1. [Working with Notifications](app/Src/notify_basics.c): one task pokes another
   with `xTaskNotifyGive` and the receiver wakes on `ulTaskNotifyTake`, a
   notification used as a binary semaphore.
2. [Sending a Value](app/Src/notify_value.c): the sender delivers a 32-bit value
   with `xTaskNotify` and the receiver reads it with `xTaskNotifyWait`, a mailbox
   without a queue.
3. [Counting Events](app/Src/notify_counting.c): a burst of gives accumulates and
   the receiver drains them one at a time with `ulTaskNotifyTake(pdFALSE, ...)`,
   a counting semaphore.
4. [Setting Bits](app/Src/notify_bits.c): two senders OR different bits into one
   receiver with `eSetBits`, a one-to-one event group.
5. [Notifying from an ISR](app/Src/notify_isr.c): the button's EXTI interrupt
   notifies a task with `vTaskNotifyGiveFromISR`, the only direction that crosses
   the interrupt boundary.