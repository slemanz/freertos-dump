/*
 * Example: task creation.
 * The simplest FreeRTOS example: create three independent tasks and let
 * the scheduler run them. Each task increments its own counter, which can
 * be inspected with a debugger to confirm all of them are executing.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

/* Each task is a function that never returns (infinite loop inside). */
void vBlueLedControllerTask(void *pvParameters);
void vRedLedControllerTask(void *pvParameters);
void vGreenLedControllerTask(void *pvParameters);

/* Profilers: counters used to observe (via debugger) that each task runs. */
typedef uint32_t TaskProfiler;

TaskProfiler BlueTaskProfiler, RedTaskProfiler, GreenTaskProfiler;

int main(void)
{
    config_app();

    /*
     * xTaskCreate arguments:
     *   1) task function
     *   2) descriptive name (for debugging)
     *   3) stack size (in words, not bytes)
     *   4) parameter passed to the task (none here -> NULL)
     *   5) priority (same value -> round-robin between tasks)
     *   6) handle to the created task (not needed here -> NULL)
     */
    xTaskCreate(vBlueLedControllerTask,
                "Blue Led Controller",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vGreenLedControllerTask,
                "Green Led Controller",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vRedLedControllerTask,
                "Red Led Controller",
                100,
                NULL,
                1,
                NULL);

    /* Hand control over to the scheduler; it never returns. */
    vTaskStartScheduler();

    /* Only reached if there was not enough heap to start the scheduler. */
    while(1)
    {
    }
}

/*
 * Each task loops forever. vTaskDelay blocks the task for the given time,
 * yielding the CPU so the other tasks can run.
 */
void vBlueLedControllerTask(void *pvParameters)
{
    while(1)
    {
        BlueTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vRedLedControllerTask(void *pvParameters)
{
    while(1)
    {
        RedTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vGreenLedControllerTask(void *pvParameters)
{
    while(1)
    {
        GreenTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}