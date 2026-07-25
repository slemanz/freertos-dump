/*
 * Example: sending more complex data with queues.
 * A queue item does not have to be a scalar. As long as every item is the
 * same fixed size, it can be a struct. Here we pass a struct by value: a
 * copy of the whole struct is placed into the queue.
 *
 * Two sender tasks put a struct carrying their own id and a counter into a
 * single queue. One receiver reads whatever arrives and toggles a different
 * LED depending on which sender it came from.
 *
 * (For large payloads you would send a POINTER to the data instead of a copy,
 * to save memory and time - but then you must guarantee the data outlives the
 * trip through the queue.)
 */

 #include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

/* Identifies which sender produced the item */
typedef enum
{
    SENDER_RED = 0,
    SENDER_GREEN
}SenderId_t;

/* The fixed-size item carried by the queue. */
typedef struct
{
    SenderId_t id;
    uint32_t value;
}Message_t;

void vSenderTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

static QueueHandle_t xQueue = NULL;

/* Seeds passed toeach sender task by pointer; they never change. */
static const Message_t xRedSeed   = {SENDER_RED,   0 };
static const Message_t xGreenSeed = {SENDER_GREEN, 0 };

int main(void)
{
    config_app();

    /* The queue now holds whole Message_t structs. */
    xQueue = xQueueCreate(5, sizeof(Message_t));

    if(xQueue != NULL)
    {
        /* The receiver drives both LEDs, so init them here */
        led_init(GPIOB, LED_RED);
        led_init(GPIOB, LED_GREEN);

        /* One function reused for two senders; the seed tells them apart. */
        xTaskCreate(vSenderTask, "Sender Red",   100, (void*)&xRedSeed,   1, NULL);
        xTaskCreate(vSenderTask, "Sender Green", 100, (void*)&xGreenSeed, 1, NULL);
        xTaskCreate(vReceiverTask, "Receiver",   100, NULL,               1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Reused by both senders. The seed passed as parameter says which one it is. */
void vSenderTask(void *pvParameters)
{
    /* Take a private copy of the seed to work with */
    Message_t msg = *(const Message_t *)pvParameters;
    TickType_t xDelay;

    /* Red sends faster than green, so its LED blinks faster. */
    xDelay = (msg.id == SENDER_RED) ? pdMS_TO_TICKS(300) : pdMS_TO_TICKS(700);

    while(1)
    {
        xQueueSend(xQueue, &msg, portMAX_DELAY);
        msg.value++;
        vTaskDelay(xDelay);
    }
}

/* Reads structs from the shared queue and acts on the id field. */
void vReceiverTask(void *pvParameters)
{
    Message_t msg;

    while(1)
    {
        if(xQueueReceive(xQueue, &msg, portMAX_DELAY) ==pdPASS)
        {
            if(msg.id == SENDER_RED)
            {
                GPIO_ToggleOutputPin(GPIOB, LED_RED);
            }else
            {
                GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
            }
        }
    }
}