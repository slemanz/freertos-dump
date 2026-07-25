/*
 * Example: working with counting semaphores.
 * A counting semaphore holds a COUNT instead of a flag. Every give
 * increments it, every take decrements it, and a task only blocks when the
 * count reaches zero.
 *
 *   xSemaphoreCreateCounting(maxCount, initialCount)
 *
 * Two classic uses:
 *   - COUNTING EVENTS (this example): start at 0. Events arriving faster
 *     than they can be handled accumulate instead of being lost.
 *   - MANAGING A RESOURCE POOL: start the count at the size of the pool,
 *     e.g. xSemaphoreCreateCounting(3, 3) for three identical buffers.
 *     Takers succeed while a slot is free and block when all are in use.
 *
 * This is the direct contrast with sem_binary_isr.c: same button, same ISR,
 * but there a burst of presses collapsed into one, and here every press is
 * accounted for.
 *
 * Requires configUSE_COUNTING_SEMAPHORES set to 1 (already 1 here).
 */

 #include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5
#define BUTTON_PIN          GPIO_PIN_NO_0

void vWorkerTask(void *pvParameters);

static SemaphoreHandle_t xCountingSemaphore = NULL;

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    /* Up to 10 pending events, none pending at startup. */
    xCountingSemaphore = xSemaphoreCreateCounting(10, 0);

    if(xCountingSemaphore != NULL)
    {
        button_init(GPIOA, BUTTON_PIN, GPIO_MODE_IT_FT);
        interrupt_SetPriority(IRQ_NO_EXTI0, 6);
        interrupt_Config(IRQ_NO_EXTI0, ENABLE);

        xTaskCreate(vWorkerTask, "Worker", 300, NULL, 2, NULL);

        vTaskStartScheduler();
    }
}

/* Same tiny ISR as before: it just increments the count. */
void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    EXTI->PR |= (1 << BUTTON_PIN);

    xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * Handles one event per pass and takes a whole second doing it, so a burst
 * of presses necessarily piles up behind it.
 */
void vWorkerTask(void *pvParameters)
{
    uint32_t ulHandled = 0;
    UBaseType_t uxPending;

    led_init(GPIOB, LED_YELLOW);
    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        if(xSemaphoreTake(xCountingSemaphore, portMAX_DELAY) == pdTRUE)
        {
            ulHandled++;

            /* Yellow on = busy processing one event */
            GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, 1);

            /* How many presses are still to be served. */
            uxPending = uxSemaphoreGetCount(xCountingSemaphore);

            printf("Event %lu handled, %lu still pending\r\n",
                   (unsigned long)ulHandled,
                   (unsigned long)uxPending);

            vTaskDelay(pdMS_TO_TICKS(1000));

            GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, 0);
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        }
    }
}