/*
 * Example: waiting for ANY bit (OR).
 * The same xEventGroupWaitBits, but with xWaitForAllBits = pdFALSE: the waiter
 * unblocks as soon as ANY of the requested bits is set. The return value is the
 * bit mask at that moment, so the waiter checks it to learn WHICH event woke it.
 *
 * A periodic task sets BIT_TICK every 2 s; a polled button on PA0 sets
 * BIT_BUTTON on each press. Either one wakes the waiter.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5
#define BUTTON_PIN          GPIO_PIN_NO_0

#define BIT_TICK            (1 << 0)
#define BIT_BUTTON          (1 << 1)
#define ANY_BITS            (BIT_TICK | BIT_BUTTON)

static EventGroupHandle_t xEventGroup = NULL;

void vWaiterTask(void *pvParameters);
void vTickTask(void *pvParameters);
void vButtonTask(void *pvParameters);

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_YELLOW);
    led_init(GPIOB, LED_GREEN);
    button_init(GPIOA, BUTTON_PIN, GPIO_MODE_IN);

    xEventGroup = xEventGroupCreate();

    if(xEventGroup != NULL)
    {
        xTaskCreate(vWaiterTask, "Waiter", 300, NULL, 2, NULL);
        xTaskCreate(vTickTask,   "Tick",   200, NULL, 1, NULL);
        xTaskCreate(vButtonTask, "Button", 200, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Wakes on either bit; the returned mask says which one(s). */
void vWaiterTask(void *pvParameters)
{
    EventBits_t uxBits;

    while(1)
    {
        uxBits = xEventGroupWaitBits(xEventGroup, ANY_BITS, pdTRUE, pdFALSE, portMAX_DELAY);

        if(uxBits & BIT_TICK)
        {
            GPIO_ToggleOutputPin(GPIOB, LED_YELLOW);
            printf("woke on: tick\r\n");
        }

        if(uxBits & BIT_BUTTON)
        {
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
            printf("woke on: button\r\n");
        }
    }
}

void vTickTask(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
        xEventGroupSetBits(xEventGroup, BIT_TICK);
    }
}

/* Polls PA0; pressed reads 0 (pull-up + button to ground). */
void vButtonTask(void *pvParameters)
{
    uint8_t ucLast = 1;
    uint8_t ucNow;

    while(1)
    {
        ucNow = GPIO_ReadFromInputPin(GPIOA, BUTTON_PIN);

        if((ucLast == 1) && (ucNow == 0))
        {
            xEventGroupSetBits(xEventGroup, BIT_BUTTON);
        }

        ucLast = ucNow;
        vTaskDelay(pdMS_TO_TICKS(20)); /* poll + debounce */
    }

}