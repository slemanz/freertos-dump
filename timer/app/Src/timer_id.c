/*
 * Example: one callback for many timers, told apart by the timer ID.
 * Every timer can carry a void* "ID", set when it is created (or later with
 * vTimerSetTimerID). Inside the callback, pvTimerGetTimerID(xTimer) reads it
 * back, so a single callback function can serve any number of timers and
 * still know which one just expired.
 *
 * Three auto-reload timers share vBlinkCallback. Each one stores the LED pin
 * it owns as its ID, so the callback simply toggles whatever pin fired. The
 * three LEDs end up blinking at three different rates from one function.
 */
#include <stdio.h>
#include <stdint.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

static TimerHandle_t xTimerRed = NULL;
static TimerHandle_t xTimerYellow = NULL;
static TimerHandle_t xTimerGreen = NULL;

/* Shared by all three timers. The ID holds the LED pin to toggle */
void vBlinkCallback(TimerHandle_t xTimer)
{
    uint8_t pin = (uint8_t)(uintptr_t)pvTimerGetTimerID(xTimer);
    GPIO_ToggleOutputPin(GPIOB, pin);
}

int main(void)
{
    config_app();

    led_init(GPIOB, LED_RED);
    led_init(GPIOB, LED_YELLOW);
    led_init(GPIOB, LED_GREEN);

    /* The 4th argument (the ID) is the LED pin, passed as a void*. */
    xTimerRed    = xTimerCreate("Red",    pdMS_TO_TICKS(250),  pdTRUE, (void*)(uintptr_t)LED_RED,    vBlinkCallback);
    xTimerYellow = xTimerCreate("Yellow", pdMS_TO_TICKS(500),  pdTRUE, (void*)(uintptr_t)LED_YELLOW, vBlinkCallback);
    xTimerGreen  = xTimerCreate("Green",  pdMS_TO_TICKS(1000), pdTRUE, (void*)(uintptr_t)LED_GREEN, vBlinkCallback);

    if((xTimerRed != NULL) && (xTimerYellow != NULL) && (xTimerGreen) != NULL)
    {
        xTimerStart(xTimerRed, 0);
        xTimerStart(xTimerYellow, 0);
        xTimerStart(xTimerGreen, 0);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}