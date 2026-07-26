/*
 * Example: working with software timers.
 * A software timer runs a function (its "callback") at a point in the
 * future, or repeatedly at a fixed period. It needs no task of its own:
 * the kernel's timer service task runs the callback for you.
 *
 *   xTimerCreate(name, periodInTicks, autoReload, id, callback)
 *       -> create it; it is born DORMANT (not running)
 *   xTimerStart(timer, ticksToWait)
 *       -> move it to the RUNNING state
 *
 * With autoReload = pdTRUE the timer restarts itself after every expiry, so
 * the callback runs periodically forever. There is no user task here at all;
 * the LED is toggled entirely from the timer callback.
 *
 * Requires configUSE_TIMERS set to 1 in FreeRTOSConfig.h (already 1 here).
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED_RED             GPIO_PIN_NO_3

static TimerHandle_t xBlinkTimer = NULL;

/* Runs in the timer service task. Keep it short and never block here. */
void vBlinkCallback(TimerHandle_t xTimer)
{
    GPIO_ToggleOutputPin(GPIOB, LED_RED);
}

int main(void)
{
    config_app();

    led_init(GPIOB, LED_RED);

    /* name, period (500 ms), auto-reload, id (unused), callback */
    xBlinkTimer = xTimerCreate("Blink",
                                pdMS_TO_TICKS(500),
                                pdTRUE,
                                NULL,
                                vBlinkCallback);

    if(xBlinkTimer != NULL)
    {
        /* Queue the start command; it takes effect once the scheduler runs. */
        xTimerStart(xBlinkTimer, 0);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}