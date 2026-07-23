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

static void led_init(GPIO_RegDef_t *pGPIOx, uint8_t pin)
{
    GPIO_PinConfig_t led;
    led.pGPIOx = pGPIOx;
    led.GPIO_PinNumber = pin;
    led.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinSpeed = GPIO_SPEED_FAST;
    led.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    led.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    led.GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN;
    GPIO_Init(&led);
}

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
