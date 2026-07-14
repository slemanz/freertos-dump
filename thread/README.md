# Thread Management

A FreeRTOS application is structured as a set of tasks, and almost everything you
do begins with creating one. This chapter covers how a task comes into being, how
the scheduler takes over, and how tasks can spawn further tasks at run time. The
runnable example lives in [`app/Src/main.c`](app/Src/main.c) and is built on the
project skeleton described under [Template](../template/README.md).

## Creating a Task

A task is created with `xTaskCreate`, which allocates the task's stack and control
block and registers it with the scheduler:

```c
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
                        const char *const pcName,
                        uint16_t usStackDepth,
                        void *pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t *pxCreatedTask );
```

Each parameter has a clear role. `pvTaskCode` is the function that implements the
task, an infinite loop that never returns. `pcName` is a human-readable label
used only for debugging. `usStackDepth` sizes the task's private stack, counted
in words rather than bytes. `pvParameters` is an arbitrary pointer passed
straight through to the task, which makes it easy to reuse one function across
several tasks. `uxPriority` sets the scheduling priority, with higher values
winning the CPU. Finally, `pxCreatedTask` can receive a handle to the new task,
or `NULL` when you never need to refer to it again.

The task function itself follows a fixed shape: configure whatever it owns, then
loop forever, yielding the CPU with a blocking call such as `vTaskDelay` so that
lower-priority tasks get their turn.

```c
void vTaskLED(void *pvParameters)
{
    /* one-time setup ... */
    while (1)
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

## Starting the Scheduler

Creating tasks does not run them. In `main`, the tasks are registered and then
control is handed to the kernel with `vTaskStartScheduler()`, which selects the
highest-priority ready task and never returns:

```c
int main(void)
{
    config_app();

    xTaskCreate(vTaskLED, "LED", 1000, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) { }   /* only reached if the scheduler runs out of heap */
}
```

## Creating Tasks from Other Tasks

`xTaskCreate` is not restricted to `main`; a task that is already running can
create more tasks just as easily. Because the scheduler is live, the new tasks
become eligible to run immediately, and you must *not* call
`vTaskStartScheduler()` a second time.

```c
void vBlueLedControllerTask(void *pvParameters)
{
    xTaskCreate(vRedLedControllerTask,   "Red Led Controller",   100, NULL, 1, NULL);
    xTaskCreate(vGreenLedControllerTask, "Green Led Controller", 100, NULL, 1, NULL);

    while (1)
    {
        BlueTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

This pattern is common when one task acts as a supervisor that brings the others
online only once some precondition, a peripheral finishing its startup, say, has
been met.

## Apps

1. Task Creation
2. Tasks Leds
3. Task Parameters
4. Task Priority
