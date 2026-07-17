/*
 * Example: suspending a task.
 * vTaskSuspend() takes a task out of the scheduler completely: it is no
 * longer considered to run, no matter its priority, until it is resumed.
 * A task can suspend another one (passing its handle) or itself (NULL).
 *
 * Requires INCLUDE_vTaskSuspend set to 1 in FreeRTOSConfig.h.
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

/* Handle of the task we want to suspend saved by xTaskCreate */
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
                "Controller Task",
                100,
                NULL,
                2,
                NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

/* Blinks until someone suspends it. It has no idea it will be suspended */
void vRedLedTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vControllerTask(void *pvParameters)
{
    led_init(GPIOB, LED_GREEN);

    /* Let the red task blink freely for 5 seconds */
    vTaskDelay(pdMS_TO_TICKS(5000));

    /*
     * Suspend the red task by its handle: its LED freezes in whatever
     * state it was. To suspend itself a task would call vTaskSuspend(NULL)
     */
    vTaskSuspend(xRedLedHandle);

    /* Keep blinking to prove the scheduler is still running. */
    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}