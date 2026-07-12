#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

void vTaskLED(void *pvParameters) {
    GPIO_PinConfig_t pinLed;
    pinLed.pGPIOx = GPIOA;
    pinLed.GPIO_PinNumber = GPIO_PIN_NO_5;
    pinLed.GPIO_PinMode = GPIO_MODE_OUT;
    pinLed.GPIO_PinSpeed = GPIO_SPEED_FAST;
    pinLed.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    pinLed.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    pinLed.GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN;
    GPIO_Init(&pinLed);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    config_app();

    xTaskCreate(vTaskLED, "LED", 1000, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {
    }
}
