/*
 * Example: task priority and preemption.
 * The scheduler always runs the highest priority task that is ready. Here
 * a HIGH priority task preempts a LOW priority one whenever it is ready,
 * and only lets the low task run while it is blocked inside vTaskDelay.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_HIGH            GPIO_PIN_NO_3   /* HIGH priority task */
#define LED_LOW             GPIO_PIN_NO_5   /* LOW priority task */

/*
 * Priorities: the HIGHER the number, the HIGHER the priority.
 * The ready task with the highest priority always runs first (preemption).
 */
#define PRIORITY_HIGH       2
#define PRIORITY_LOW        1

void vHighPriorityTask(void *pvParameters);
void vLowPriorityTask(void *pvParameters);

/*
 * Profilers: counters to observe (via debugger) how many times
 * each task has run. The high priority task dominates the CPU.
 */
typedef uint32_t TaskProfiler;
TaskProfiler HighTaskProfiler, LowTaskProfiler;

static const uint8_t high_led = LED_HIGH;
static const uint8_t low_led  = LED_LOW;

int main(void)
{
    config_app();

    xTaskCreate(vHighPriorityTask,
                "High Priority Task",
                100,
                (void *)&high_led,
                PRIORITY_HIGH,
                NULL);

    xTaskCreate(vLowPriorityTask,
                "Low Priority Task",
                100,
                (void *)&low_led,
                PRIORITY_LOW,
                NULL);

    vTaskStartScheduler();

    while(1)
    {
    }
}

/*
 * HIGH priority task.
 * While it is ready, it always runs instead of the low priority one.
 * vTaskDelay puts the task into the "Blocked" state, yielding the CPU
 * so that the low priority task can execute.
 */
void vHighPriorityTask(void *pvParameters)
{
    uint8_t pin = *(const uint8_t *)pvParameters;

    led_init(GPIOB, pin);

    while(1)
    {
        HighTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, pin);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/*
 * LOW priority task.
 * Only runs during the windows in which the high priority task is
 * blocked (inside its vTaskDelay).
 */
void vLowPriorityTask(void *pvParameters)
{
    uint8_t pin = *(const uint8_t *)pvParameters;

    led_init(GPIOB, pin);

    while(1)
    {
        LowTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, pin);
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}
