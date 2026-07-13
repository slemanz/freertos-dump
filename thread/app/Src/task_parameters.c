#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vLedControllerTask(void *pvParameters);

static const uint8_t red_led    = LED_RED;
static const uint8_t yellow_led = LED_YELLOW;
static const uint8_t green_led  = LED_GREEN;

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

    xTaskCreate(vLedControllerTask,
                "Red Led Controller",
                100,
                (void *)&red_led,
                1,
                NULL);

    xTaskCreate(vLedControllerTask,
                "Yellow Led Controller",
                100,
                (void *)&yellow_led,
                1,
                NULL);

    xTaskCreate(vLedControllerTask,
                "Green Led Controller",
                100,
                (void *)&green_led,
                1,
                NULL);

    vTaskStartScheduler();

    while(1)
    {
    }
}

void vLedControllerTask(void *pvParameters)
{
    uint8_t pin = *(const uint8_t *)pvParameters;

    led_init(GPIOB, pin);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, pin);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}