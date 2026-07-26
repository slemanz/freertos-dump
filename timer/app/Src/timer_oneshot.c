/*
 * Example: one-shot vs auto-reload timers.
 * The third argument of xTimerCreate picks the type:
 *
 *   pdTRUE  -> auto-reload: restarts itself after every expiry (periodic)
 *   pdFALSE -> one-shot:    fires exactly once, then goes DORMANT and stays
 *              there until something starts it again by hand
 *
 * Here an auto-reload timer blinks the green LED forever, while a one-shot
 * timer lights the red LED exactly once, ~3 s after boot, and then never
 * again. Watching the two side by side makes the difference obvious.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

static TimerHandle_t xAutoTimer = NULL;
static TimerHandle_t xOneShotTimer = NULL;

/* Auto-reload: called every 500ms, forever */
void vAutoCallback(TimerHandle_t xTimer)
{
    GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
}

/* One-shot: called a single time, 3 s after the timer is started */
void vOneShotCallback(TimerHandle_t xTimer)
{
    GPIO_ToggleOutputPin(GPIOB, LED_RED);
}

int main(void)
{
    config_app();

    led_init(GPIOB, LED_GREEN);
    led_init(GPIOB, LED_RED);

    xAutoTimer    = xTimerCreate("Auto", pdMS_TO_TICKS(500),  pdTRUE,  NULL, vAutoCallback);
    xOneShotTimer = xTimerCreate("Shot", pdMS_TO_TICKS(3000), pdFALSE, NULL, vOneShotCallback);

    if((xAutoTimer != NULL) && (xOneShotTimer != NULL))
    {
        xTimerStart(xAutoTimer, 0);
        xTimerStart(xOneShotTimer, 0);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}