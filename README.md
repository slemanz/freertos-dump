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

## RTOS vs ISR and Busy Wait Systems

Real-Time Operating Systems (RTOS) provide a structured, deterministic approach
to managing multiple tasks concurrently, contrasting sharply with the simpler,
yet less efficient, busy-wait systems and the more foundational, but limited,
Interrupt Service Routine (ISR)-only systems. 

| Feature | Busy Wait (Polling) | ISR (Interrupts) | RTOS |
| --- | --- | --- | --- |
| **CPU Utilization** | 100% (mostly wasted) | Low (only when needed) | Medium (scheduling overhead) |
| **Complexity** | Very Low | Moderate | High |
| **Responsiveness** | Slow (depends on loop size) | Very Fast | Fast & Deterministic |
| **Multitasking** | Poor | Basic (Foreground/Background) | Excellent |
| **Predictability** | Low | High for one task | Very High for many tasks |

## The Concept of a Thread

In the context of a Real-Time Operating System (RTOS), a thread (often referred
to as a task) is the smallest unit of execution managed by the RTOS scheduler.
It represents a single, sequential flow of control, allowing complex software
applications to be broken down into smaller, manageable, and concurrent units. 

## List of Contents

- [Introduction](intro)
- [Thread Management](thread)

---

Using **FreeRTOS-Kernel-11.2.0**