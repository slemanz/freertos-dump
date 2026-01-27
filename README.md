# FreeRTOS Dump

RTOS:

- Resource manager
- Guarantees deadlines

**Deadline:** deriving the right results with in a set time constraint.

**Reliability:** running in predictable way with a guaranteed response.

Kernel (the OS core):

- Thrad scheduling
- Booting
- Inter-thread communication
- syncronization

## Context Switching

When the RTOS decides to stop Task A and start Task B, it must take a perfect
snapshot of Task A’s current state so it can return to it later without Task A
ever knowing it was interrupted. This snapshot is called the Context.

Context switching in an RTOS is the process of saving the current task's state
(registers, stack pointer, program counter) and restoring the state of the next
task to run, managed by the scheduler, typically triggered by timer interrupts
(SysTick) or preemptive events.


