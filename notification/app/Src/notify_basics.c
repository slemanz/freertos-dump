/*
 * Example: working with task notifications.
 * A notification is a signal sent straight from one task to another, with no
 * queue, semaphore, or event group in between. Every task has one built-in
 * notification, so nothing is allocated: it is the fastest, cheapest way for
 * two tasks to talk.
 *
 *   xTaskNotifyGive(handle)                 -> "poke" the target task
 *   ulTaskNotifyTake(pdTRUE, ticksToWait)   -> block until poked, then clear
 *
 * Used this way it behaves like a binary semaphore. A notification has two
 * states: pending (it was given and not yet read) and not pending. Taking it
 * with pdTRUE clears the value back to zero, so it is a plain on/off signal.
 *
 * Requires configUSE_TASK_NOTIFICATIONS (default 1).
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

/* The sender needs the handle of the task it notifies. */
static TaskHandle_t xReceiverHandle = NULL;

void vSenderTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

int main(void)
{
    config_app();

    led_init(GPIOB, LED_RED);
    led_init(GPIOB, LED_GREEN);

    xTaskCreate(vReceiverTask, "Receiver", 200, NULL, 2, &xReceiverHandle);
    xTaskCreate(vSenderTask,   "Sender",   200, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vSenderTask(void *pvParameters)
{
    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        xTaskNotifyGive(xReceiverHandle);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vReceiverTask(void *pvParameters)
{
    while(1)
    {
        /* Blocks with no CPU cost until the sender pokes it. */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
    }
}