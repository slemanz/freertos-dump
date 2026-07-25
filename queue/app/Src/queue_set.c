/*
 * Example: working with queue sets.
 * A queue set lets one task wait on several queues at once, without polling
 * each of them in turn. xQueueSelectFromSet() blocks until ANY member queue
 * has data, and returns the handle of the one that did. You then read from
 * that queue as usual.
 *
 * Two senders write to two separate queues at different rates. A single
 * receiver waits on the set and toggles the LED matching whichever queue
 * delivered.
 *
 * Requires configUSE_QUEUE_SETS set to 1 in FreeRTOSConfig.h.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vRedSenderTask(void *pvParameters);
void vGreenSenderTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

static QueueHandle_t xRedQueue    = NULL;
static QueueHandle_t xGreenQueue  = NULL;
static QueueSetHandle_t xQueueSet = NULL;

int main(void)
{
    config_app();

    xRedQueue   = xQueueCreate(1, sizeof(uint32_t));
    xGreenQueue = xQueueCreate(1, sizeof(uint32_t));

    /**
     * The set must be bigenough to hold the sum of the lengths of every
     * queue added to it: 1 + 1 = 2.
     */
    xQueueSet = xQueueCreateSet(1 + 1);

    if(xRedQueue != NULL && xGreenQueue != NULL && xQueueSet != NULL)
    {
        /* A queue can only join a set while it is empty. */
        xQueueAddToSet(xRedQueue,   xQueueSet);
        xQueueAddToSet(xGreenQueue, xQueueSet);

        /* The receiver drives both LEDs, so init them here */
        led_init(GPIOB, LED_RED);
        led_init(GPIOB, LED_GREEN);

        xTaskCreate(vRedSenderTask,   "Red Sender",   100, NULL, 1, NULL);
        xTaskCreate(vGreenSenderTask, "Green Sender", 100, NULL, 1, NULL);
        xTaskCreate(vReceiverTask,    "Receiver",     100, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Sends to the red queue every 400 ms. */
void vRedSenderTask(void *pvParameters)
{
    uint32_t ulValue = 0;

    while(1)
    {
        xQueueSend(xRedQueue, &ulValue, 0);
        ulValue++;
        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

/* Sends to the green queue every 900 ms. */
void vGreenSenderTask(void *pvParameters)
{
    uint32_t ulValue = 0;

    while(1)
    {
        xQueueSend(xGreenQueue, &ulValue, 0);
        ulValue++;
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

/*
 * Waits on the whole set. xQueueSelectFromSet returns the handle of the
 * queue that has data; we then read from exactly that queue. Because the
 * set told us it is ready, the receive is guaranteed to succeed at once.
 */
void vReceiverTask(void *pvParameters)
{
    QueueSetMemberHandle_t xActivated;
    uint32_t ulReceived;

    while(1)
    {
        xActivated = xQueueSelectFromSet(xQueueSet, portMAX_DELAY);
        
        if(xActivated == xRedQueue)
        {
            xQueueReceive(xRedQueue, &ulReceived, 0);
            GPIO_ToggleOutputPin(GPIOB, LED_RED);
        }else if(xActivated == xGreenQueue)
        {
            xQueueReceive(xGreenQueue, &ulReceived, 0);
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        }
    }
}