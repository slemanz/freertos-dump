/*
 * Example: working with a mutex.
 * A mutex (MUTual EXclusion) guarantees that only one task at a time uses
 * a shared resource. Here the shared resource is the serial port.
 *
 *   xSemaphoreCreateMutex()  -> create it (starts AVAILABLE, unlike binary)
 *   xSemaphoreTake(...)      -> lock
 *   xSemaphoreGive(...)      -> unlock
 *
 * A mutex differs from a binary semaphore in two ways:
 *   - it is created available, so the first take succeeds immediately;
 *   - it has an OWNER: the task that takes it is the one that must give it
 *     back. Use a semaphore to SIGNAL, a mutex to LOCK.
 *
 * Set USE_MUTEX to 0 to watch the two tasks corrupt each other's output.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define USE_MUTEX           1

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vPrintTask(void *pvParameters);

static SemaphoreHandle_t xMutex = NULL;

/* Labels passed to the two tasks as their parameter. */
static const char *pcRedLabel = "[RED]";
static const char *pcGreenLabel = "[GREEN]";

/**
 * Prints one slow line, on porpuse, one character at a time. The delay
 * between characters widens the window in which another task can barge in,
 * making the race obvious on the terminal.
 */
static void print_slow_line(const char *pcLabel)
{
    uint8_t i;

    printf("%s", pcLabel);

    for(i = 0; i < 10; i++)
    {
        printf("%u", i);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    printf("\r\n");
}

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    xMutex = xSemaphoreCreateMutex();

    if(xMutex != NULL)
    {
        led_init(GPIOB, LED_RED);
        led_init(GPIOB, LED_YELLOW);
        led_init(GPIOB, LED_GREEN);

        /* Same function, same priority: they really do interleave. */
        xTaskCreate(vPrintTask, "Print Red",   300, (void*)pcRedLabel,   1, NULL);
        xTaskCreate(vPrintTask, "Print Green", 300, (void*)pcGreenLabel, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Reused by both tasks; the label passed as parameter tells them apart. */
void vPrintTask(void *pvParameters)
{
    const char *pcLabel = (const char *)pvParameters;

    while(1)
    {
#if(USE_MUTEX == 1)
        /* only one task at time may be inside this block */
        if(xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            print_slow_line(pcLabel);

            /* ALWAYS give it back, on every path out of the section */
            xSemaphoreGive(xMutex);
        }
#else
        /*unprotected: both task write to the same UART at once. */
        print_slow_line(pcLabel);
#endif

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}