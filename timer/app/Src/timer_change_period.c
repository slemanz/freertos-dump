/*
 * Example: changing a timer's period at runtime from the potentiometer.
 * xTimerChangePeriod(timer, newPeriodInTicks, ticksToWait) sets a new period
 * and, as a side effect, starts the timer if it happened to be dormant. It
 * also restarts the countdown from zero.
 *
 * An auto-reload timer blinks the red LED. A task reads the potentiometer on
 * PA1 (a 12-bit value, 0..4095) and maps it to a blink period, so turning the
 * knob speeds the LED up or slows it down live.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_adc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED_RED             GPIO_PIN_NO_3

#define PERIOD_MIN_MS       50
#define PERIOD_MAX_MS       1050

static TimerHandle_t xBlinkTimer = NULL;

void vPotTask(void *pvParameters);

void vBlinkCallback(TimerHandle_t xTimer)
{
    GPIO_ToggleOutputPin(GPIOB, LED_RED);
}

int main(void)
{
    config_app();

    led_init(GPIOB, LED_RED);
    adc_pa1_init();
    adc_start_conversion();

    xBlinkTimer = xTimerCreate("Blink", pdMS_TO_TICKS(PERIOD_MIN_MS), pdTRUE, NULL, vBlinkCallback);

    if(xBlinkTimer != NULL)
    {
        xTimerStart(xBlinkTimer, 0);

        xTaskCreate(vPotTask, "Pot", 200, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Maps the 12-bit ADC reading onto [PERIOD_MIN_MS, PERIOD_MAX_MS]. */
void vPotTask(void *pvParameters)
{
    uint32_t ulRaw;
    uint32_t ulPeriodMs;
    uint32_t ulLastMs = 0;

    while(1)
    {
        ulRaw = adc_read();  /* 0 .. 4095 */

        ulPeriodMs = PERIOD_MIN_MS + (ulRaw * (PERIOD_MAX_MS - PERIOD_MIN_MS)) / 4095;

        /*
         * Only push a new period when the knob actually moved. Because
         * xTimerChangePeriod() restarts the countdown, calling it on every
         * pass would keep resetting the timer and it would never expire at
         * long periods.
         */
        if((ulPeriodMs > ulLastMs + 10) || (ulPeriodMs + 10 < ulLastMs))
        {
            xTimerChangePeriod(xBlinkTimer, pdMS_TO_TICKS(ulPeriodMs), pdMS_TO_TICKS(10));
            ulLastMs = ulPeriodMs;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

