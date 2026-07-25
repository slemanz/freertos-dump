/*
 * Example: synchronizing tasks with queues.
 * A queue is not only a data pipe, it is also a synchronization tool. A task
 * that blocks on xQueueReceive stays in the Blocked state (using no CPU)
 * until another task sends something. The receiver's pace is driven entirely
 * by the sender, not by a timer of its own.
 *
 * Here the producer signals the consumer once every 2 seconds. The consumer
 * has no vTaskDelay: it only ever wakes when the producer sends. Stop the
 * producer and the consumer stops too - it is synchronized to it.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vProducerTask(void *pvParameters);
void vConsumerTask(void *pvParameters);

static QueueHandle_t xSyncQueue = NULL;

int main(void)
{
    config_app();

    /* The item is just a signal; its value does not matter herer */
    xSyncQueue = xQueueCreate(1, sizeof(uint8_t));

    if(xSyncQueue != NULL)
    {
        xTaskCreate(vProducerTask, "Producer", 100, NULL, 1, NULL);
        xTaskCreate(vConsumerTask, "Consumer", 100, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Every 2 s, toggles the red LED and then signals the consumer. */
void vProducerTask(void *pvParameters)
{
    uint8_t ucSignal = 1;

    led_init(GPIOB, LED_RED);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));

        GPIO_ToggleOutputPin(GPIOB, LED_RED);

        /* Wake the consumer. The value carried is irrelevant. */
        xQueueSend(xSyncQueue, &ucSignal, portMAX_DELAY);
    }
}

/* Has no delay of its own: it runs only when the producer signals it. */
void vConsumerTask(void *pvParameters)
{
    uint8_t ucSignal;

    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        /* Blocked here until the producer sends. This IS the synchronization. */
        if(xQueueReceive(xSyncQueue, &ucSignal, portMAX_DELAY) == pdPASS)
        {
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        }
    }
}