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

