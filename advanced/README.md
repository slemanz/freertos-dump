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
