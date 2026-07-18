/*
 * Example: terminating a task.
 * vTaskDelete() removes a task permanently. It can delete another task by
 * handle, or itself by passing NULL - in that case the call never returns.
 *
 * Two important rules:
 *   - A task function must NEVER just return or fall off its end. It must
 *     loop forever or delete itself with vTaskDelete(NULL).
 *   - The memory of a deleted task is freed by the Idle task, so the Idle
 *     task must be given some CPU time.
 *
 * Requires INCLUDE_vTaskDelete set to 1 in FreeRTOSConfig.h.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vRedLedTask(void *pvParameters);
void vYellowLedTask(void *pvParameters);
void vGreenLedTask(void *pvParameters);

/* Handle of the task that will be deleted from the outside. */
static TaskHandle_t xGreenLedHandle = NULL;

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
                NULL);

    xTaskCreate(vYellowLedTask,
                "Yellow Led Task",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vGreenLedTask,
                "Green Led Task",
                100,
                NULL,
                1,
                &xGreenLedHandle);
    
    vTaskStartScheduler();

    while(1)
    {

    }
}

/**
 * Blinks forever and, after 5 seconds, deletes the green task by handle.
 * After deleting a task, its handle is dangling: never use it again.
 */
void vRedLedTask(void *pvParameters)
{
    uint32_t count = 0;

    led_init(GPIOB, LED_RED);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        count++;

        if(count == 10) /* 10 x 500 ms = 5 s*/
        {
            vTaskDelete(xGreenLedHandle);
            xGreenLedHandle = NULL;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* Blink 20 times, then delete itself */
void vYellowLedTask(void *pvParameters)
{
    uint32_t blinks = 0;

    led_init(GPIOB, LED_YELLOW);

    while(blinks < 20)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_YELLOW);
        blinks++;
        vTaskDelay(pdMS_TO_TICKS(250));
    }

    /* Leave the LED off, then terminate. This call never returns */
    GPIO_WriteToOutputPin(GPIOB, LED_YELLOW, 0);
    vTaskDelete(NULL);
}

/* Knows nothing about being deleted; it is killed by the red task. */
void vGreenLedTask(void *pvParameters)
{
    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}