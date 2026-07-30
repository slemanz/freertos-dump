# Under the Hood

The chapters so far have treated FreeRTOS through its API: create a task, send
to a queue, take a semaphore. This closing chapter looks beneath that surface
at four things the API quietly depends on: the Cortex-M hardware features the
port is built on, the heap that every `Create` call draws from, the hook
functions through which the kernel calls back into the application, and the
CMSIS-RTOS layer that vendor tools wrap around the native API. It is a
reference chapter, all prose and no apps. The scheduling policies themselves,
preemption, time slicing, and cooperative scheduling, have their own chapter
under [Understanding the Scheduler](../scheduler/README.md).

## Cortex-M OS Support Features

The Cortex-M core was designed with an operating system in mind, and the
FreeRTOS port leans on that support rather than working around the hardware.
Four features carry most of the weight:

- **Two stack pointers.** The core banks two stack pointers behind the one
  `sp` register: the *main* stack pointer (MSP) and the *process* stack
  pointer (PSP). FreeRTOS runs every task on its own stack through the PSP,
  while exception handlers, including the kernel itself, run on the MSP. The
  practical payoff is that a task's stack never has to reserve room for
  interrupt handling, so task stacks can be sized for the task alone.
- **SysTick.** A 24-bit down-counting timer built into the core itself, not
  the vendor's peripheral set, which is why the same port runs on every
  Cortex-M part. FreeRTOS programs it to fire the tick interrupt; in this
  repository it runs at 1000 Hz, one tick per millisecond.
- **PendSV.** A *pendable* exception held at the lowest interrupt priority,
  and the place where the context switch actually happens. The tick handler,
  or an API call that wakes a higher-priority task, does not switch tasks on
  the spot; it merely *pends* PendSV. The hardware then takes the exception
  only once no other interrupt is active, so a context switch can never cut
  an ISR in half, and interrupt handling never has to be context-switch aware.
- **SVC.** The *supervisor call* instruction raises an exception on demand.
  The port uses it exactly once, to launch the first task when
  `vTaskStartScheduler` hands control to the kernel.

The context switch itself splits the work with the hardware. On any exception
entry the core automatically stacks `xPSR`, `PC`, `LR`, `R12`, and `R0`-`R3`
onto the running task's stack. The PendSV handler saves the remaining
`R4`-`R11`, stores the task's stack pointer into its task control block, loads
the stack pointer of the task chosen by the scheduler, and unstacks in reverse
order. The suspended task never knows it was gone; that is the *context* of
context switching.

One more feature ties into interrupts: the `BASEPRI` register lets the kernel
mask interrupts only up to `configMAX_SYSCALL_INTERRUPT_PRIORITY` rather than
disabling them globally. Interrupts above that threshold never feel the
kernel's critical sections but must not call the RTOS API; the rules, and the
`configASSERT` trap waiting for those who break them, are covered in
[Interrupt Management](../interrupt/README.md).

## Heap Memory Management

Every `xTaskCreate`, `xQueueCreate`, and `xTimerCreate` in this repository
allocates its stacks and control blocks from the FreeRTOS heap through
`pvPortMalloc` and `vPortFree`, not the toolchain's `malloc` and `free`. The
standard library allocator is a poor fit for real-time work: it is rarely
thread-safe, its timing is not deterministic, and its bookkeeping overhead is
outside your control. FreeRTOS instead treats allocation as part of the
*portable* layer and ships five interchangeable implementations under
[`freertos/portable/MemMang`](../freertos/portable/MemMang); you compile
exactly one of them into the image. Apart from heap_3, they all serve memory
from a statically declared array of `configTOTAL_HEAP_SIZE` bytes, 15 KB in
this project's `FreeRTOSConfig.h`.

- **heap_1** only allocates and never frees. `vPortFree` does nothing, so
  fragmentation is impossible and every allocation is deterministic. It fits
  the common embedded pattern where every object is created at boot and lives
  forever.
- **heap_2** adds `vPortFree` with a best-fit search, but never merges
  adjacent free blocks back together. Repeatedly allocating varied sizes
  fragments its heap, and it survives only for backward compatibility;
  heap_4 supersedes it.
- **heap_3** wraps the compiler's own `malloc` and `free`, made thread-safe
  by suspending the scheduler around each call. The heap lives wherever the
  linker put it, and `configTOTAL_HEAP_SIZE` has no effect.
- **heap_4** uses a first-fit search and, crucially, *coalesces* adjacent
  free blocks into one, which lets it survive repeated create/delete cycles
  without fragmenting. It is the general-purpose choice, and the one this
  repository builds; see `FREERTOS_OBJS` in the template's
  [`Makefile`](../template/app/Makefile).
- **heap_5** is heap_4 extended to serve several non-contiguous memory
  regions as one heap, described to the kernel with
  `vPortDefineHeapRegions` before the first allocation. It exists for parts
  whose RAM is scattered across the address map.

Two query functions make sizing empirical instead of guesswork:
`xPortGetFreeHeapSize` reports the heap remaining now, and
`xPortGetMinimumEverFreeHeapSize` reports the low-water mark since boot, the
number that tells you how close a run actually came to exhaustion.

The heap is also optional. With `configSUPPORT_STATIC_ALLOCATION` set to 1,
each kernel object gains a `Static` twin, `xTaskCreateStatic`,
`xQueueCreateStatic`, and the rest, where the caller supplies the buffers and
control block. Nothing is allocated at runtime, every byte is visible at link
time, and allocation cannot fail, which is why safety-critical projects often
forbid the dynamic API outright.

## Hook Functions

Hooks are functions with fixed names that the application chooses to provide
and the kernel calls at defined moments; each is enabled by a switch in
`FreeRTOSConfig.h`. The two everyday ones, the idle hook and the tick hook,
were introduced in [Thread Management](../thread/README.md). The two that
remain are error hooks, and a dump is exactly where they belong, because both
catch failures that are otherwise silent.

The **malloc failed hook** answers the question of what happens when the heap
runs dry. Without it, a failed `pvPortMalloc` simply makes the API call fail,
`xTaskCreate` returns `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY`, and a program
that does not check return values limps on missing a task. With
`configUSE_MALLOC_FAILED_HOOK` set to 1, the kernel instead calls
`vApplicationMallocFailedHook()` at the moment of failure, one place to trap,
log, or halt.

The **stack overflow hook** catches the classic embedded failure of a task
outgrowing its stack. `configCHECK_FOR_STACK_OVERFLOW` selects the detection
method: 1 checks the task's stack pointer at each context switch out, cheap
but blind to overflows that grow and shrink between switches; 2 additionally
fills the stack with a known pattern at creation and checks that its last 16
bytes are intact, slower but far harder to slip past. Either way the kernel
calls `vApplicationStackOverflowHook(xTask, pcTaskName)` with the offender's
handle and name. By then the damage is done, adjacent memory may already be
corrupt, so treat it as a development aid, not a recovery path. Its
constructive partner is `uxTaskGetStackHighWaterMark`, which reports how close
a task has ever come to the bottom of its stack; measure with a generous
stack, then trim with a margin.

A third, rarer hook rounds out the set: with
`configUSE_DAEMON_TASK_STARTUP_HOOK` set to 1, the kernel calls
`vApplicationDaemonTaskStartupHook()` once, when the timer service task first
runs, a convenient spot for initialization that must wait until the scheduler
is up.

## CMSIS-RTOS

CMSIS-RTOS is not another operating system: it is Arm's standard RTOS *API*,
a vendor-neutral wrapper so that middleware and application code can be
written once and run over any kernel that implements it. FreeRTOS is one such
kernel, through Arm's CMSIS-FreeRTOS adapter, and that pairing is what
STM32CubeMX generates when you tick the FreeRTOS box: the project it emits
calls `osKernelInitialize` and `osThreadNew`, not `xTaskCreate`, with the
native kernel running underneath. Two API versions exist, CMSIS-RTOS v1 and
the current v2 (`cmsis_os2.h`); new code targets v2.

Everything in this repository uses the native API deliberately, the wrapper
is a thin layer, and knowing the native calls means you can read straight
through it. The mapping is close to mechanical:

| CMSIS-RTOS2 | FreeRTOS native |
| --- | --- |
| `osKernelInitialize` / `osKernelStart` | setup / `vTaskStartScheduler` |
| `osThreadNew` | `xTaskCreate` |
| `osDelay` | `vTaskDelay` |
| `osMessageQueueNew` / `Put` / `Get` | `xQueueCreate` / `Send` / `Receive` |
| `osSemaphoreNew` / `Acquire` / `Release` | `xSemaphoreCreate...` / `Take` / `Give` |
| `osMutexNew` | `xSemaphoreCreateMutex` |
| `osTimerNew` / `osTimerStart` | `xTimerCreate` / `xTimerStart` |
| `osEventFlagsSet` / `Wait` | `xEventGroupSetBits` / `WaitBits` |
| `osThreadFlagsSet` / `Wait` | `xTaskNotify` / `xTaskNotifyWait` |

The vocabulary shifts, CMSIS says *thread* where FreeRTOS says *task*, and
*event flags* for *event groups*, and priorities are expressed on the
`osPriority_t` scale (`osPriorityLow` up through `osPriorityRealtime`) that
the adapter maps onto the 0 to `configMAX_PRIORITIES - 1` range; both count
upward, higher number meaning higher priority. When a Cube-generated project
misbehaves, the fastest route is usually to translate the `os` calls back to
this table and reason about the native kernel you already know.