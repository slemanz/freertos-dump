/*
 * Example: setting bits in a notification (a mini event group).
 * With the eSetBits action, a notification carries a set of flags, just like an
 * event group -- but one-to-one, since a notification has a single receiver.
 *
 *   xTaskNotify(handle, bits, eSetBits)                 -> OR these bits in
 *   xTaskNotifyWait(0, 0xFFFFFFFF, &bits, ticksToWait)  -> block, read, clear
 *
 * Two senders set BIT_ONE and BIT_TWO at their own pace. If both arrive before
 * the receiver runs, the bits accumulate and the receiver sees them together.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

#define BIT_ONE             (1 << 0)
#define BIT_TWO             (1 << 1)

static TaskHandle_t xReceiverHandle = NULL;

void vSenderOneTask(void *pvParameters);
void vSenderTwoTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_YELLOW);
    led_init(GPIOB, LED_GREEN);

    xTaskCreate(vReceiverTask,  "Receiver", 300, NULL, 2, &xReceiverHandle);
    xTaskCreate(vSenderOneTask, "SenderOne", 200, NULL, 1, NULL);
    xTaskCreate(vSenderTwoTask, "SenderTwo", 200, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vSenderOneTask(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1500));
        xTaskNotify(xReceiverHandle, BIT_ONE, eSetBits);
    }
}

void vSenderTwoTask(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(2300));
        xTaskNotify(xReceiverHandle, BIT_TWO, eSetBits);
    }
}

void vReceiverTask(void *pvParameters)
{
    uint32_t ulBits;

    while(1)
    {
        if(xTaskNotifyWait(0, 0xFFFFFFFF, &ulBits, portMAX_DELAY) == pdTRUE)
        {
            if(ulBits & BIT_ONE)
            {
                GPIO_ToggleOutputPin(GPIOB, LED_YELLOW);
                printf("bit one\r\n");
            }

            if(ulBits & BIT_TWO)
            {
                GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
                printf("bit two\r\n");
            }
        }
    }
}