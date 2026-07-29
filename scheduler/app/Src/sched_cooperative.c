/*
 * Example: cooperative scheduling.
 * With configUSE_PREEMPTION at 0 the scheduler never forces a task out: a task
 * runs until it voluntarily gives up the CPU, either by blocking (vTaskDelay,
 * waiting on a queue, ...) or by calling taskYIELD() explicitly. Even a
 * higher-priority task will not run until the current one lets go.
 *
 * Two tasks take turns only because each one calls taskYIELD() after its work.
 * Delete the yield from one of them and it will hog the CPU forever.
 *
 * Requires configUSE_PREEMPTION set to 0 in FreeRTOSConfig.h.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vTaskPing(void *pvParameters);
void vTaskPong(void *pvParameters);

/* Bounded busy loop: burns time without blocking, so it stays cooperative. */
static void busy_wait(volatile uint32_t n)
{
    while(n > 0)
    {
        n--;
    }
}

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_RED);
    led_init(GPIOB, LED_GREEN);

    xTaskCreate(vTaskPing, "Ping", 300, NULL, 1, NULL);
    xTaskCreate(vTaskPong, "Pong", 300, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vTaskPing(void *pvParameters)
{
    while(1)
    {
        printf("ping\r\n");
        GPIO_ToggleOutputPin(GPIOB, LED_RED);

        busy_wait(2000000);
        taskYIELD();   /* hand the CPU over on purpose */
    }
}

void vTaskPong(void *pvParameters)
{
    while(1)
    {
        printf("pong\r\n");
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);

        busy_wait(2000000);
        taskYIELD();
    }
}
