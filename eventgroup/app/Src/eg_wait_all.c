/*
 * Example: one event group, multiple setters, one waiter (AND).
 * Several tasks set bits into the SAME event group; a waiter blocks until a
 * whole COMBINATION of bits is present. This is where an event group replaces
 * a fistful of binary semaphores.
 *
 * xEventGroupWaitBits(group, bits, xClearOnExit, xWaitForAllBits, ticks):
 *   xWaitForAllBits = pdTRUE -> unblock only when ALL of 'bits' are set (AND)
 *   xClearOnExit    = pdTRUE -> clear those bits on the way out, so it re-arms
 *
 * Three worker tasks each finish at their own pace and set their own bit. The
 * waiter fires only once all three have reported in, toggles an LED, and the
 * cycle repeats.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define LED_GREEN           GPIO_PIN_NO_5

#define BIT_A               (1 << 0)
#define BIT_B               (1 << 1)
#define BIT_C               (1 << 2)
#define ALL_BITS            (BIT_A | BIT_B | BIT_C)

typedef struct
{
    EventBits_t bit;
    const char *name;
    uint32_t ms;
}Worker_t;

static const Worker_t workers[3] =
{
    { BIT_A, "A", 500  },
    { BIT_B, "B", 900  },
    { BIT_C, "C", 1400 },
};

static EventGroupHandle_t xEventGroup = NULL;

void vWorkerTask(void *pvParameters);
void vWaiterTask(void *pvParameters);

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_GREEN);

    xEventGroup = xEventGroupCreate();

    if(xEventGroup != NULL)
    {
        xTaskCreate(vWaiterTask, "Waiter", 300, NULL, 2, NULL);
        xTaskCreate(vWorkerTask, "Worker A", 200, (void*)&workers[0], 1, NULL);
        xTaskCreate(vWorkerTask, "Worker B", 200, (void*)&workers[1], 1, NULL);
        xTaskCreate(vWorkerTask, "Worker C", 200, (void*)&workers[2], 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Each worker takes its own time, then reports in by setting its bit. */
void vWorkerTask(void *pvParameters)
{
    const Worker_t *w = (const Worker_t *)pvParameters;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(w->ms)); /* simulate doing the work */

        printf("Worker %s done\r\n", w->name);
        xEventGroupSetBits(xEventGroup, w->bit);
    }
}

/* Blocks until ALL three bits are set, then clears them (re-arms) and fires. */
void vWaiterTask(void *pvParameters)
{
    while(1)
    {
        xEventGroupWaitBits(xEventGroup, ALL_BITS, pdTRUE, pdTRUE, portMAX_DELAY);

        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        printf(">>> all three workers reported, round complete\r\n");
    }
}