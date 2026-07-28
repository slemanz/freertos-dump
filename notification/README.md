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