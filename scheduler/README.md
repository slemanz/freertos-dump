# Understanding the Scheduler

The scheduler is the routine at the heart of the kernel that decides which
Ready task moves into the Running state, and when. Everything else in FreeRTOS,
tasks, priorities, blocking, exists so the scheduler can make that decision
predictably. This chapter looks at the rules it follows and, crucially, at the
few configuration switches that change them, because the scheduler's behavior
*is* its configuration. The runnable examples live under [`app/Src/`](app/Src/),
one file per concept, and are built on the project skeleton described under
[Template](../template/README.md).

Unlike the other chapters, several examples here require editing
`FreeRTOSConfig.h` and rebuilding. This project ships with
`configUSE_PREEMPTION` set to `1` and no `configUSE_TIME_SLICING` line, so it
defaults to `1`: prioritized, preemptive, time-sliced. The tick runs at 1000 Hz,
so a time slice is one millisecond.