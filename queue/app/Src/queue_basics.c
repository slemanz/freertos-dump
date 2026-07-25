/*
 * Example: working with queues.
 * A queue is a FIFO buffer that holds a fixed number of fixed-size items.
 * Data is inserted at the back and removed from the front. It is the main
 * way tasks pass data to each other safely, without sharing variables.
 *
 * Here one task sends an incrementing number every second and another task
 * receives it and toggles an LED. The receiver blocks on the queue, so it
 * uses no CPU while waiting for data.
 *
 *   xQueueCreate(length, itemSize)      -> create the queue
 *   xQueueSend(q, &item, ticksToWait)   -> write one item at the back
 *   xQueueReceive(q, &buf, ticksToWait) -> read one item from the front
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_GREEN           GPIO_PIN_NO_5

void vSenderTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

/* Handle of the queue shared by the two tasks. */
static QueueHandle_t xQueue = NULL;

/* Last value received, kept for inspection in the debugger. */
static uint32_t ulLastReceived = 0;

int main(void)
{
    config_app();

    /* Room for 5 items, each the size of a uint32_t */
    xQueue = xQueueCreate(5, sizeof(uint32_t));

    /* Create the tasks only if the queue was actually created. */
    if(xQueue != NULL)
    {
        xTaskCreate(vSenderTask,   "Sender",   100, NULL, 1, NULL);
        xTaskCreate(vReceiverTask, "Receiver", 100, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Sends an incrementing counter into the queue once per second. */
void vSenderTask(void *pvParameters)
{
    uint32_t ulValue = 0;

    while(1)
    {
        /* Send a copy ulValue to the back of the queue. */
        xQueueSend(xQueue, &ulValue, portMAX_DELAY);
        ulValue++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* Blocks until an item arrives, then toggles the green LED. */
void vReceiverTask(void *pvParameters)
{
    uint32_t ulReceived;

    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        /**
         * portMAX_DELAY: wait forever until data is available. The task
         * stays Blocked (no CPU used) until the sender writes something.
         */
        if(xQueueReceive(xQueue, &ulReceived, portMAX_DELAY) == pdPASS)
        {
            ulLastReceived = ulReceived;
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        }
    }
}