/*
 * Example: multiple LED tasks.
 * Three separate tasks, one per LED, each blinking at a different rate.
 * Same priority, so the scheduler shares the CPU between them (round-robin),
 * and every LED blinks independently.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

/* One dedicated task function per LED. */
void vRedLedControllerTask(void *pvParameters);
void vYellowLedControllerTask(void *pvParameters);
void vGreenLedControllerTask(void *pvParameters);

/* Profilers: counters to observe (via debugger) how often each task runs. */
typedef uint32_t TaskProfiler;

TaskProfiler RedTaskProfiler, YellowTaskProfiler, GreenTaskProfiler;

int main(void)
{
    config_app();

    xTaskCreate(vRedLedControllerTask,
                "Red Led Controller",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vYellowLedControllerTask,
                "Yellow Led Controller",
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

    vTaskStartScheduler();

    while(1)
    {
    }
}

/* Each task blinks its LED at a different rate, set by its vTaskDelay. */

/* Red: toggles every 1000 ms -> slowest blink. */
void vRedLedControllerTask(void *pvParameters)
{
    led_init(GPIOB, GPIO_PIN_NO_3);

    while(1)
    {
        RedTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, GPIO_PIN_NO_3);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Yellow: toggles every 500 ms -> medium blink. */
void vYellowLedControllerTask(void *pvParameters)
{
    led_init(GPIOB, GPIO_PIN_NO_4);

    while(1)
    {
        YellowTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, GPIO_PIN_NO_4);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* Green: toggles every 250 ms -> fastest blink. */
void vGreenLedControllerTask(void *pvParameters)
{
    led_init(GPIOB, GPIO_PIN_NO_5);

    while(1)
    {
        GreenTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, GPIO_PIN_NO_5);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
