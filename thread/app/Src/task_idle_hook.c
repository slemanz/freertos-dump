/*
 * Example: working with the Idle task.
 * The Idle task is created automatically by vTaskStartScheduler(). It runs
 * at priority 0 (the lowest) whenever no other task is ready, and it is
 * responsible for freeing the memory of deleted tasks.
 *
 * With configUSE_IDLE_HOOK set to 1, the kernel calls vApplicationIdleHook()
 * on every pass of the Idle task. Use it for low priority background work
 * or to measure how idle the CPU is.
 *
 * Rules for the idle hook:
 *   - It must NEVER block or call any API that can block (no vTaskDelay).
 *   - It must always return, and it must be fast.
 *   - If the application ever deletes tasks, it must not starve the Idle
 *     task, or memory is never reclaimed.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vRedLedTask(void *pvParameters);
void vCpuHogTask(void *pvParameters);

/**
 * Counts how many times the Idle task ran. It stops growing while the CPU
 * is busy: a rough measure of a spare CPU time.
 */
typedef uint32_t TaskProfiler;
TaskProfiler IdleTaskProfiler;

int main(void)
{
    config_app();

    /* The green LED is driven by the idle hook, so init it here */
    led_init(GPIOB, LED_GREEN);

    xTaskCreate(vRedLedTask,
                "Red Led Task",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vCpuHogTask,
                "Cpu Hog Task",
                100,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

/*
 * Called by the Idle task whenever nothing else is ready to run.
 * Toggling this fast makes the green LED look dim: the CPU is idle.
 * When the LED freezes, the CPU is fully busy.
 */
void vApplicationIdleHook(void)
{
    IdleTaskProfiler++;
    GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
}

/* A well behaved task: blocks most of the time, leaving the CPU idle */
void vRedLedTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/*
 * Every 4 seconds, hogs the CPU for about 2 seconds at a higher priority.
 * While the yellow LED is on, the Idle task never runs.
 */
void vCpuHogTask(void *pvParameters)
{
    volatile uint32_t i;

    led_init(GPIOB, LED_YELLOW);

    while(1)
    {
        /* Idle CPU: green LED looks dim. */
        vTaskDelay(pdMS_TO_TICKS(4000));

        /* Busy CPU: green LED freezes while yellow is on */
        GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, 1);

        for(i = 0; i < 4000000; i++)
        {

        }

        GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, 0);
    }
}
