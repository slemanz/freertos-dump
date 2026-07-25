/*
 * Example: working with binary semaphores.
 * A semaphore is a signal, or a key: it carries no data, it only says
 * "something happened" or "you may proceed".
 *
 * A binary semaphore holds a single flag: taken or available.
 *
 *   xSemaphoreCreateBinary()          -> create it (starts EMPTY!)
 *   xSemaphoreTake(sem, ticksToWait)  -> wait for / grab the key
 *   xSemaphoreGive(sem)               -> hand the key over
 *
 * Because it is created empty, the taker blocks until the first give.
 * And because it is a single flag, giving twice before it is taken is the
 * same as giving once: the second give is simply lost. That is exactly the
 * limitation counting semaphores exist to fix (see sem_counting.c).
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vGiverTask(void *pvParameters);
void vTakerTask(void *pvParameters);

static SemaphoreHandle_t xBinarySemaphore = NULL;

int main(void)
{
    config_app();

    xBinarySemaphore = xSemaphoreCreateBinary();

    if(xBinarySemaphore != NULL)
    {
        xTaskCreate(vGiverTask, "Giver", 100, NULL, 1, NULL);
        xTaskCreate(vTakerTask, "Taker", 100, NULL, 2, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Every second, signals the taker by giving the semaphore. */
void vGiverTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        GPIO_ToggleOutputPin(GPIOB, LED_RED);

        /* Hand the key over. Nobory "owns" a binary semaphore. */
        xSemaphoreGive(xBinarySemaphore);
    }

}

/**
 * Has a felay of its own: it runs only when it manages to take the key.
 * It is created at a HIGHER priority than the giver, so as soons as the
 * semaphore is given it preempts the giver and runs immediately.
 */
void vTakerTask(void *pvParameters)
{
    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        /* Blocked here (no CPU used) until the giver gives */
        if(xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        }
    }
}