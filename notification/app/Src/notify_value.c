/*
 * Example: sending a value with a notification (a mailbox).
 * The notification is not just a flag: it holds a 32-bit value. With the
 * eSetValueWithOverwrite action the sender writes that value directly into the
 * receiver's notification, replacing whatever was there.
 *
 *   xTaskNotify(handle, value, eSetValueWithOverwrite)   -> deliver a value
 *   xTaskNotifyWait(0, 0xFFFFFFFF, &value, ticksToWait)  -> block, read, clear
 *
 * This is the "faster and lighter than a queue" case from the lesson: one
 * writer, one reader, one 32-bit word, no queue object.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_GREEN           GPIO_PIN_NO_5

static TaskHandle_t xReceiverHandle = NULL;

void vSenderTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_GREEN);

    xTaskCreate(vReceiverTask, "Receiver", 300, NULL, 2, &xReceiverHandle);
    xTaskCreate(vSenderTask,   "Sender",   200, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vSenderTask(void *pvParameters)
{
    uint32_t ulValue = 0;

    while(1)
    {
        ulValue++;
        xTaskNotify(xReceiverHandle, ulValue, eSetValueWithOverwrite);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vReceiverTask(void *pvParameters)
{
    uint32_t ulReceived;

    while(1)
    {
        /* Wait for a value; clear all bits on exit so each read starts clean. */
        if(xTaskNotifyWait(0, 0xFFFFFFFF, &ulReceived, portMAX_DELAY) == pdTRUE)
        {
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
            printf("got value: %lu\r\n", (unsigned long)ulReceived);
        }
    }
}