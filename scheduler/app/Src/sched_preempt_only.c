/*
 * Example: prioritized preemptive scheduling WITHOUT time slicing.
 * The scheduler always runs the highest-priority ready task, and preempts the
 * running task the instant a higher-priority one becomes ready. But it does NOT
 * share time between tasks of EQUAL priority: with configUSE_TIME_SLICING at 0,
 * a running task keeps the CPU until it blocks, yields, or is preempted by
 * something more important.
 *
 * Three equal-priority workers spin in a tight loop. With time slicing off, one
 * of them monopolizes the CPU and the other two never run. A higher-priority
 * reporter blocks on vTaskDelay, so it still runs and prints the counters.
 *
 * Requires configUSE_TIME_SLICING set to 0 in FreeRTOSConfig.h.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

static const uint8_t leds[3] = { GPIO_PIN_NO_3, GPIO_PIN_NO_4, GPIO_PIN_NO_5 };
static volatile uint32_t ulCounter[3] = { 0, 0, 0 };

void vWorkerTask(void *pvParameters);
void vReporterTask(void *pvParameters);

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, leds[0]);
    led_init(GPIOB, leds[1]);
    led_init(GPIOB, leds[2]);

    /* Three workers at the SAME priority. */
    xTaskCreate(vWorkerTask, "WorkerA", 200, (void*)0, 1, NULL);
    xTaskCreate(vWorkerTask, "WorkerB", 200, (void*)1, 1, NULL);
    xTaskCreate(vWorkerTask, "WorkerC", 200, (void*)2, 1, NULL);

    /* Reporter above them: it blocks, so it always gets to run. */
    xTaskCreate(vReporterTask, "Reporter", 300, NULL, 2, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

/* CPU-bound: never blocks. Counts and blinks its LED once in a while. */
void vWorkerTask(void *pvParameters)
{
    uint32_t ulIndex = (uint32_t)pvParameters;

    while(1)
    {
        ulCounter[ulIndex]++;

        if((ulCounter[ulIndex] % 400000U) == 0U)
        {
            GPIO_ToggleOutputPin(GPIOB, leds[ulIndex]);
        }
    }

}

/* Prints the three counters once a second. */
void vReporterTask(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("A=%lu  B=%lu  C=%lu\r\n",
               (unsigned long)ulCounter[0],
               (unsigned long)ulCounter[1],
               (unsigned long)ulCounter[2]);
    }
}