# FreeRTOS Dump

A Real-Time Operating System (RTOS) is, at heart, a resource manager. On a
microcontroller with a single core, only one instruction stream can run at a
time, yet real firmware must juggle many responsibilities at once: blink a
status LED, service a UART, sample a sensor, and answer a button, all seemingly
in parallel. The RTOS creates that illusion by slicing the CPU between
independent tasks and deciding, moment to moment, which one runs.

What separates an RTOS from ordinary multitasking is the "real-time" guarantee,
and that guarantee is about predictability, not raw speed. A well-designed
real-time system derives the right result *within a bounded time*, and it does
so reliably: it responds to events in a way you can reason about and count on
before the deadline arrives. Determinism, not throughput, is the product.

That behavior is delivered by the kernel, the small core at the center of the
operating system. The kernel owns the machinery the rest of the system takes for
granted: bringing the chip up at boot, scheduling which task runs next, switching
between them, and giving tasks safe ways to communicate and synchronize with one
another. The chapters here build that picture up one concept at a time, each
backed by code that compiles and runs on an STM32F411.

## Context Switching

When the scheduler decides to stop task A and start task B, it must capture a
perfect snapshot of task A's state so that A can later resume exactly where it
left off, never aware it was interrupted. That snapshot, the CPU registers, the
stack pointer, and the program counter, is what we call the *context*.

Context switching is the act of saving the outgoing task's context and restoring
the incoming task's. It is driven by the scheduler and typically triggered either
by a periodic timer interrupt (the SysTick) or by a preemptive event that makes a
higher-priority task ready to run. The cost of that save-and-restore is the price
of multitasking; a good RTOS keeps it small and, just as importantly, constant.

## RTOS, Interrupts, and Busy-Wait Loops

Before reaching for an RTOS it is worth understanding what it buys you over the
two simpler structures it competes with. A **busy-wait** (polling) design loops
forever, checking each condition in turn; it is trivial to write but burns the
CPU doing nothing useful and reacts only as fast as the loop is short. Moving
work into **interrupt service routines** fixes responsiveness for a handful of
events, but coordinating many of them by hand quickly becomes unmanageable. An
**RTOS** accepts a modest scheduling overhead in exchange for deterministic,
structured multitasking across many tasks.

| Feature | Busy Wait (Polling) | ISR (Interrupts) | RTOS |
| --- | --- | --- | --- |
| **CPU Utilization** | 100% (mostly wasted) | Low (only when needed) | Medium (scheduling overhead) |
| **Complexity** | Very Low | Moderate | High |
| **Responsiveness** | Slow (depends on loop size) | Very Fast | Fast & Deterministic |
| **Multitasking** | Poor | Basic (Foreground/Background) | Excellent |
| **Predictability** | Low | High for one task | Very High for many tasks |

## The Concept of a Thread

Within an RTOS, a *thread*, more often called a *task*, is the smallest unit of
execution the scheduler manages. Each task is a single, sequential flow of
control with its own stack, written as if it owned the processor: usually an
initialization step followed by an infinite loop. Breaking an application into
several small, focused tasks, rather than one monolithic loop, is what lets a
complex system stay readable while the kernel handles the concurrency underneath.

## Contents

In reading order:

- [Introduction](intro/README.md): the FreeRTOS naming conventions and core data
  types you meet everywhere in the API.
- [Thread Management](thread/README.md): creating tasks, starting the scheduler,
  and spawning tasks from within other tasks.
- [Template](template/README.md): the bare-metal STM32F411 project skeleton the
  examples are built on, with drivers, linker, and build wired up.

---

Built against **FreeRTOS-Kernel-11.2.0**.
