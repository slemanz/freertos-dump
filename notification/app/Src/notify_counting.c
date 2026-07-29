/*
 * Example: counting events with a notification (a counting semaphore).
 * Each xTaskNotifyGive increments the notification value. Taking it with
 * pdFALSE decrements it by ONE instead of clearing it, so the count is kept:
 *
 *   xTaskNotifyGive(handle)         -> value++ (one event)
 *   ulTaskNotifyTake(pdFALSE, wait) -> value--, returns the value BEFORE the --
 *
 * The sender bursts three gives at once; the receiver drains them one at a
 * time, so no event is lost. That is exactly what a counting semaphore does.
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

    led_init(GPIOB, LED_GREEN);

    xTaskCreate(vReceiverTask, "Receiver", 200, NULL, 2, &xReceiverHandle);
    xTaskCreate(vSenderTask,   "Sender",   200, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

/* Every 2 s, fire three events in a quick burst. */
void vSenderTask(void *pvParameters)
{
    while(1)
    {
        xTaskNotifyGive(xReceiverHandle);
        xTaskNotifyGive(xReceiverHandle);
        xTaskNotifyGive(xReceiverHandle);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }

}

/* Drains one event per loop; the count makes sure all three are handled. */
void vReceiverTask(void *pvParameters)
{
    while(1)
    {
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(150)); /* slow enough to see each toggle */
    }

}