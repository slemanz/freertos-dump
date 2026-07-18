/*
 * Example: executing a task periodically.
 * vTaskDelay(x) waits x ticks COUNTED FROM THE MOMENT IT IS CALLED, so the
 * real period becomes (work time + x) and drifts as the work time varies.
 *
 * xTaskDelayUntil(&xLastWakeTime, xPeriod) wakes at a FIXED interval,
 * absolute in time, no matter how long the work took. This is what you
 * want for real periodic tasks (sampling, control loops).
 *
 * Requires INCLUDE_vTaskDelayUntil set to 1 in FreeRTOSConfig.h.
 * (vTaskDelayUntil is the older name of the same API.)
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vDriftingTask(void *pvParameters);
void vPeriodicTask(void *pvParameters);
void vSecondPeriodicTask(void *pvParameters);

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

/* Simulates work that takes a variable amount of time */
static void do_some_work(void)
{
    volatile uint32_t i;

    for(i = 0; i < 200000; i++)
    {
    }
}

int main(void)
{
    config_app();

    xTaskCreate(vDriftingTask,
                "Drifting Task",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vPeriodicTask,
                "Periodic Task",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vSecondPeriodicTask,
                "Second Periodic Task",
                100,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();

    while (1)
    {
    }
}

/* Relative delay: real period = work time + 500 ms -> it drifts. */
void vDriftingTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        do_some_work();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* Absolute delay: wakes exactly every 500 ms -> no drift. */
void vPeriodicTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    led_init(GPIOB, LED_GREEN);

    /**
     * Initialize ONCE with the current tick count, before the loop.
     * xTaskDelayUntil updates it automatically on every call.
     */
    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        do_some_work();
        xTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void vSecondPeriodicTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    led_init(GPIOB, LED_YELLOW);
    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_YELLOW);
        do_some_work();
        xTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}