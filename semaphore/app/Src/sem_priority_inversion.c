/*
 * Example: priority inversion and priority inheritance.
 *
 * Three tasks and one shared resource:
 *   LOW    (prio 1) - takes the resource and holds it for a while
 *   MEDIUM (prio 2) - wants NOTHING, just burns CPU
 *   HIGH   (prio 3) - needs the same resource as LOW
 *
 * PRIORITY INVERSION: HIGH blocks waiting for a resource held by LOW. LOW
 * cannot run to release it, because MEDIUM (which does not care about the
 * resource at all) keeps preempting it. So HIGH ends up waiting on MEDIUM,
 * a task it outranks. The delay is bounded only by how long MEDIUM runs.
 *
 * PRIORITY INHERITANCE: a MUTEX fixes this. While HIGH is blocked on the
 * mutex, the kernel temporarily raises LOW to HIGH's priority so it can run,
 * finish, and give the mutex back. The promotion ends at the give.
 *
 * A binary semaphore does NOT do this. Flip USE_MUTEX and compare the
 * measured waiting times printed by the high priority task.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define USE_MUTEX           1

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vLowPriorityTask(void *pvParameters);
void vMediumPriorityTask(void *pvParameters);
void vHighPriorityTask(void *pvParameters);

static SemaphoreHandle_t xResource = NULL;

/**
 * Busy work that does NOT block: it keeps the task in the Ready/Running
 * state, which is the whole point, a vTaskDelay here would hand the CPU
 * over and the inversion would never show up.
 */
static void burn_cpu(uint32_t ulLoops)
{
    volatile uint32_t i;

    for(i = 0; i < ulLoops; i++)
    {
    }
}

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

#if(USE_MUTEX == 1)
    xResource = xSemaphoreCreateMutex();    /* Start available */
#else
    xResource = xSemaphoreCreateBinary();   /* binary starts EMPTY */
    xSemaphoreGive(xResource);
#endif

    if(xResource != NULL)
    {
        led_init(GPIOB, LED_RED);
        led_init(GPIOB, LED_YELLOW);
        led_init(GPIOB, LED_GREEN);

        xTaskCreate(vLowPriorityTask,    "Low",    200, NULL, 1, NULL);
        xTaskCreate(vMediumPriorityTask, "Medium", 200, NULL, 2, NULL);
        xTaskCreate(vHighPriorityTask,   "High",   300, NULL, 3, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Grabs the resource and holds it while doing slow, non-blocking work. */
void vLowPriorityTask(void *pvParameters)
{
    while(1)
    {
        if(xSemaphoreTake(xResource, portMAX_DELAY) == pdTRUE)
        {
            GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, 1);
            
            /*
             * Holding the resource. Without inheritance, MEDIUM preempts
             * us right here and HIGH pays for it.
             */
            burn_cpu(400000);

            GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, 0);
            xSemaphoreGive(xResource);
        }

        /* Let the others get a turn before grabbing it again. */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/*
 * The troublemaker. It never touches the resource, it just outranks LOW and
 * refuses to block, so it starves the resource holder.
 */
void vMediumPriorityTask(void *pvParameters)
{
    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        burn_cpu(2000000);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* Measures how long it actually had to wait for the resource. */
void vHighPriorityTask(void *pvParameters)
{
    TickType_t xStart, xWaited;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));

        xStart  = xTaskGetTickCount();

        if(xSemaphoreTake(xResource, portMAX_DELAY) == pdTRUE)
        {
            xWaited = xTaskGetTickCount() - xStart;

            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);

            /* ConfigTICK_RATE_HZ is 1000, so 1 tick == 1ms */
            printf("High waited %lu ms \r\n", (unsigned long)xWaited);

            xSemaphoreGive(xResource);
        }
    }
}