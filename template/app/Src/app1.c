/*
 * Example app1: blink the red LED.
 * A single task toggles the red LED (GPIOB, pin 3) every 500 ms.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3

void vRedLedTask(void *pvParameters);

int main(void)
{
    config_app();

    xTaskCreate(vRedLedTask,
                "Red Led",
                100,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();

    while(1)
    {
    }
}

/* Toggles the red LED every 500 ms. */
void vRedLedTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
