# Introduction

## Data types

**TickType_t:**

- Unsigned 16-bit type
- Unsigned 32-bit type
- (base on config settings)

**BaseType_t:**

- Unsigned 16-bit type on 16-bit architecture
- Unsigned 32-bit type on 32-bit architecture

## Variables Types

**Variables are prefixed with ther type:**

- **c**: char
- **s**: short/uint16_t
- **l**: long/int32_t
- **x**: BaseType_t and any other non-standard types (e.g structeres, task handlers, queue handles, etc).
- **u**: unsigned
- **p**: pointer
- **v**: void

## Function Names

Functions are prefixed with both the type they return, and the file they are
defined within.

E.g:

- `vTaskPrioritySet()` returns a void and is defined within *task.c*
- `xQueueReceive()` returns a variable of type BaseType_t and is defined within *queue.c*
- `pvTimerGetTimerID()` returns a pointer to void and is defined within *timers.c*

## Macro Names

- Macro in uppercase
- Prefix in lowercase
- Prefix indicates macro definition file

port**MAX_DELAY** : located in portable.h

Other examples:

- taskENTER_CRITICAL() : task.h
- pdTRUE : projdefs.h
- configUSE_PREEMPTION : FreeRTOSConfig.h
- errQUEUE_FULL : projdefs.h

| Macro | Value |
| --- | --- |
| pdTRUE | 1 |
| pdFALSE | 0 |
| pdPASS | 1 |
| pdFAIL | 0 |

