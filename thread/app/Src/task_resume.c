/*
 * Example: resuming a suspended task.
 * vTaskResume() puts a suspended task back into the Ready state, so the
 * scheduler considers it again. It is the counterpart of vTaskSuspend().
 *
 * Note: suspension is NOT counted. Several vTaskSuspend() calls are undone
 * by a single vTaskResume(). Resuming a task that is not suspended has no
 * effect.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vRedLedTask(void *pvParameters);
void vControllerTask(void *pvParameters);

static TaskHandle_t xRedLedHandle = NULL;

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
                "Red Led Task",
                100,
                NULL,
                1,
                &xRedLedHandle);
    
    xTaskCreate(vControllerTask,
                "Red Led Task",
                100,
                NULL,
                2,
                NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vRedLedTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/* Suspends and resumes the red task in a 3 s cycle, forever */
void vControllerTask(void *pvParameters)
{
    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        /* Red blinks for 3 seconds... */
        vTaskDelay(pdMS_TO_TICKS(3000));

        vTaskSuspend(xRedLedHandle);
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);

        /* ...stays frozen for 3 seconds... */
        vTaskDelay(pdMS_TO_TICKS(3000));

        /* ..and is put back into the Ready state. */
        vTaskResume(xRedLedHandle);
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
    }
}