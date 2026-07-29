/*
 * Example: suspending the scheduler (vTaskSuspendAll / xTaskResumeAll).
 * These calls lock and unlock the scheduler: while it is suspended, no context
 * switch to another TASK happens, so a section between them runs atomically with
 * respect to other tasks. Unlike a critical section (taskENTER_CRITICAL), this
 * does NOT disable interrupts -- ISRs still run; it only stops task switching.
 *
 * Two equal-priority tasks each print a three-line block. Wrapped in
 * vTaskSuspendAll/xTaskResumeAll, each block comes out whole instead of
 * interleaving with the other task's lines.
 *
 * Rule: do NOT call any blocking API (vTaskDelay, queue waits, ...) while the
 * scheduler is suspended. printf here only busy-waits on the UART, which is fine.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

void vTaskA(void *pvParameters);
void vTaskB(void *pvParameters);

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    xTaskCreate(vTaskA, "TaskA", 300, NULL, 1, NULL);
    xTaskCreate(vTaskB, "TaskB", 300, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vTaskA(void *pvParameters)
{
    while(1)
    {
        vTaskSuspendAll();  /* no task switch until resumed */
        {
            printf("A: 1\r\n");
            printf("A: 2\r\n");
            printf("A: 3\r\n");
        }
        xTaskResumeAll();

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void vTaskB(void *pvParameters)
{
    while(1)
    {
        vTaskSuspendAll();
        {
            printf("B: 1\r\n");
            printf("B: 2\r\n");
            printf("B: 3\r\n");
        }
        xTaskResumeAll();

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}