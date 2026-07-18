/*
 * Example: blocking a task.
 * A task is always in one of these states: Running, Ready, Blocked or
 * Suspended. The difference between waiting badly and waiting well:
 *
 *   - busy-wait (a software loop): the task stays READY and keeps using
 *     the CPU, starving every lower priority task.
 *   - vTaskDelay(): the task goes to the BLOCKED state, is removed from
 *     the ready list and gives the CPU to other tasks until the timeout
 *     expires.
 *
 * Here the red task busy-waits at a HIGHER priority, so the green task
 * (lower priority) is starved and its LED barely moves.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define USE_BUSY_WAIT       1

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vBusyWaitTask(void *pvParameters);
void vBlockingTask(void *pvParameters);

/* Profilers: compare how many times each task runs. */
typedef uint32_t TaskProfiler;
TaskProfiler BusyTaskProfiler, BlockedTaskProfiler;

static void led_init(GPIO_RegDef_t *pGPIOx, uint8_t pin)
{
    GPIO_PinConfig_t led;
    led.pGPIOx = pGPIOx;
    led.GPIO_PinNumber = pin;
    led.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinSpeed = GPIO_SPEED_FAST;
    led.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    led.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    led.GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN;
    GPIO_Init(&led);
}

/* Wastes CPU on purpose: never calls the kernel, never yields. */
static void busy_wait(void)
{
#if USE_BUSY_WAIT
    volatile uint32_t i;

    for(i = 0; i < 1000000; i++)
    {

    }
#else
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

int main(void)
{
    config_app();

    xTaskCreate(vBusyWaitTask,
                "Busy Wait Task",
                100,
                NULL,
                2,
                NULL);
    
    xTaskCreate(vBlockingTask,
                "Blocking Task",
                100,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

/**
 * The WRONG way to wait: stays Ready the whole time and hogs the CPU.
 * 
 * EXPERIMENT: deactive USE_BUSY_WAIT
 */
void vBusyWaitTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        BusyTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        busy_wait();
    }
}

/* The right way to wait: goes Blocked and lets others run. */
void vBlockingTask(void *pvParameters)
{
    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        BlockedTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}