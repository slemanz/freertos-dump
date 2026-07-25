/*
 * Example: task parameters.
 * Shows how to reuse ONE task function for several tasks by passing a
 * different parameter to each one. Here the parameter is the LED pin, so
 * the same vLedControllerTask drives three different LEDs.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vLedControllerTask(void *pvParameters);

/*
 * The parameter passed to a task must stay valid for the whole task life,
 * so these are 'static' (persist for the entire program) instead of local
 * variables that would go out of scope.
 */
static const uint8_t red_led    = LED_RED;
static const uint8_t yellow_led = LED_YELLOW;
static const uint8_t green_led  = LED_GREEN;


int main(void)
{
    config_app();

    /*
     * 4th argument is the parameter passed to the task. We give the address
     * of each LED pin, so the same task function controls a different LED.
     */
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

/*
 * Shared task function. pvParameters is a void pointer, so we cast it back
 * to the original type (uint8_t) and read the pin value it points to.
 */
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