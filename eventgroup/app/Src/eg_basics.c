/*
 * Example: setting and reading event bits in an event group.
 * An event group is a set of bits (flags) shared by several tasks. Each bit
 * means "some event happened". You set bits, clear bits, and read them back:
 *
 *   xEventGroupCreate()                -> make the group (all bits 0)
 *   xEventGroupSetBits(group, bits)    -> turn bits ON
 *   xEventGroupClearBits(group, bits)  -> turn bits OFF
 *   xEventGroupGetBits(group)          -> read the whole bit mask
 *
 * With configUSE_16_BIT_TICKS at 0, bits 0..23 are usable. Here a producer
 * turns three bits on one at a time and then clears them, while a monitor
 * mirrors each bit onto an LED, so you watch the flags come and go.
 *
 * Event groups are enabled by default (configUSE_EVENT_GROUPS).
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

#define BIT_RED             (1 << 0)
#define BIT_YELLOW          (1 << 1)
#define BIT_GREEN           (1 << 2)

static EventGroupHandle_t xEventGroup = NULL;

void vProducerTask(void *pvParameters);
void vMonitorTask(void *pvParameters);

int main(void)
{
    config_app();

    led_init(GPIOB, LED_RED);
    led_init(GPIOB, LED_YELLOW);
    led_init(GPIOB, LED_GREEN);

    xEventGroup = xEventGroupCreate();

    if(xEventGroup != NULL)
    {
        xTaskCreate(vProducerTask, "Producer", 200, NULL, 1, NULL);
        xTaskCreate(vMonitorTask,  "Monitor",  200, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Turns the three bits on one at a time, then clears them all, forever. */
void vProducerTask(void *pvParameters)
{
    while(1)
    {
        xEventGroupSetBits(xEventGroup, BIT_RED);
        vTaskDelay(pdMS_TO_TICKS(1000));

        xEventGroupSetBits(xEventGroup, BIT_YELLOW);
        vTaskDelay(pdMS_TO_TICKS(1000));

        xEventGroupSetBits(xEventGroup, BIT_GREEN);
        vTaskDelay(pdMS_TO_TICKS(1000));

        xEventGroupClearBits(xEventGroup, BIT_RED | BIT_YELLOW | BIT_GREEN);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Reads the whole mask and drives each LED from its bit. */
void vMonitorTask(void *pvParameters)
{
    EventBits_t uxBits;

    while(1)
    {
        uxBits = xEventGroupGetBits(xEventGroup);

        GPIO_WriteToOutputPin(GPIOB, LED_RED,    (uxBits & BIT_RED)    ? GPIO_PIN_SET : GPIO_PIN_RESET);
        GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, (uxBits & BIT_YELLOW) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        GPIO_WriteToOutputPin(GPIOB, LED_GREEN,  (uxBits & BIT_GREEN)  ? GPIO_PIN_SET : GPIO_PIN_RESET);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}