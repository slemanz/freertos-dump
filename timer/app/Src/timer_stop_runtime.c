/*
 * Example: stopping (and restarting) an auto-reload timer at runtime.
 * A running timer is not permanent. From any task you can:
 *
 *   xTimerStop(timer, ticksToWait)   -> back to DORMANT, callback stops
 *   xTimerStart(timer, ticksToWait)  -> RUNNING again
 *   xTimerIsTimerActive(timer)       -> pdTRUE while it is running
 *
 * An auto-reload timer blinks the green LED. A polled button on PA0 toggles
 * that timer on and off: press once and the blinking stops, press again and
 * it resumes. The red LED echoes each press.
 *
 * (There are also xTimerStopFromISR / xTimerStartFromISR for use inside an
 * interrupt; those follow the same FromISR + portYIELD_FROM_ISR rules as the
 * semaphore module. Here we poll from a task, so none of that is needed.)
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5
#define BUTTON_PIN          GPIO_PIN_NO_0

static TimerHandle_t xBlinkTimer = NULL;

void vButtonTask(void *pvParameters);

void vBlinkCallback(TimerHandle_t xTimer)
{
    GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
}

int main(void)
{
    config_app();

    led_init(GPIOB, LED_GREEN);
    led_init(GPIOB, LED_RED);
    button_init(GPIOA, BUTTON_PIN, GPIO_MODE_IN);

    xBlinkTimer = xTimerCreate("Blink", pdMS_TO_TICKS(200), pdTRUE, NULL, vBlinkCallback);

    if(xBlinkTimer != NULL)
    {
        xTimerStart(xBlinkTimer, 0);

        xTaskCreate(vButtonTask, "Button", 200, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/*
 * Polls PA0. On each falling edge (press), toggle the timer between running
 * and stopped. Pressed reads 0 because the pin is pulled up to VCC and the
 * button shorts it to ground.
 */
void vButtonTask(void *pvParameters)
{
    uint8_t ucLast = 1;
    uint8_t ucNow;

    while(1)
    {
        ucNow = GPIO_ReadFromInputPin(GPIOA, BUTTON_PIN);

        if((ucLast == 1) && (ucNow == 0))
        {
            GPIO_ToggleOutputPin(GPIOB, LED_RED);

            if(xTimerIsTimerActive(xBlinkTimer) == pdFALSE)
            {
                xTimerStart(xBlinkTimer, 0);
            }
            else
            {
                xTimerStop(xBlinkTimer, 0);
            }
        }

        ucLast = ucNow;
        vTaskDelay(pdMS_TO_TICKS(20)); /* poll + debounce */
    }
}