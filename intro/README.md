# Introduction to the FreeRTOS API

FreeRTOS is written to be portable across dozens of architectures and toolchains,
and that portability leaves a visible fingerprint on its API. Types are aliased
so the kernel can adapt to the target's word size, and every identifier carries a
prefix that encodes its type and origin. Once you learn to read those
conventions, an unfamiliar function name tells you what it returns and where it
lives before you ever open the header.

## Portable Data Types

Rather than use raw C types directly, FreeRTOS defines aliases whose exact width
is chosen at build time. Two of them appear constantly:

- **`TickType_t`** counts RTOS ticks. Depending on `configUSE_16_BIT_TICKS`, it
  is an unsigned 16-bit or 32-bit integer: 16 bits saves memory on small parts,
  while 32 bits pushes the tick-counter overflow far into the future.
- **`BaseType_t`** is the architecture's most efficient integer type, mapping to
  16 bits on a 16-bit MCU and 32 bits on a 32-bit core. It is the natural return
  type for functions that report a simple status or boolean.

## Variable Prefixes

Variables are prefixed with a letter naming their type, so the declaration and
every later use carry that information inline:

- **`c`** — `char`
- **`s`** — `short` / `uint16_t`
- **`l`** — `long` / `int32_t`
- **`x`** — `BaseType_t` and other non-standard types such as structures, task
  handles, and queue handles
- **`u`** — prepended for `unsigned`
- **`p`** — prepended for a pointer
- **`v`** — `void`

The prefixes combine, so an `unsigned long` becomes `ul` and a pointer to `char`
becomes `pc`.

## Function Names

A function name is prefixed with both the type it returns and the file in which
it is defined, in that order. The pattern reads like a compact signature:

- `vTaskPrioritySet()` returns `void` and lives in *tasks.c*.
- `xQueueReceive()` returns a `BaseType_t` and lives in *queue.c*.
- `pvTimerGetTimerID()` returns a `void *` and lives in *timers.c*.

## Macro Names

Macros follow a complementary rule: the body is written in uppercase while a
lowercase prefix marks the file that defines them. `portMAX_DELAY`, for example,
comes from *portable.h*, while `taskENTER_CRITICAL()` comes from *task.h* and the
configuration switch `configUSE_PREEMPTION` comes from *FreeRTOSConfig.h*. A
handful of status constants recur throughout the API:

| Macro | Value | Defined in |
| --- | --- | --- |
| `pdTRUE` | 1 | projdefs.h |
| `pdFALSE` | 0 | projdefs.h |
| `pdPASS` | 1 | projdefs.h |
| `pdFAIL` | 0 | projdefs.h |

Error codes follow the same scheme, so `errQUEUE_FULL` announces itself as an
error constant from *projdefs.h* the moment you read its name.
