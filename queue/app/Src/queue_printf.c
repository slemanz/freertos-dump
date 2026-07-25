/*
 * Example: updating printf.
 * The same producer/consumer queue as before, but now the receiver prints
 * every value it takes off the queue over the serial port (UART2, 115200).
 *
 * printf is already retargeted to UART2 in config.c (__io_putchar). We only
 * disable stdout buffering so each line goes out immediately.
 *
 * This is the usual pattern for logging: worker tasks push messages/values
 * into a queue and a single task owns the serial port and prints them, so
 * the output never gets interleaved.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_GREEN           GPIO_PIN_NO_5

void vSenderTask(void *pvParameters);
void vPrinterTask(void *pvParameters);

static QueueHandle_t xQueue = NULL;

int main(void)
{
    config_app();

    /* Send each printf out immediately, without buffering. */
    setvbuf(stdout, NULL, _IONBF, 0);

    xQueue = xQueueCreate(5, sizeof(uint32_t));

    if(xQueue != NULL)
    {
        xTaskCreate(vSenderTask,  "Sender",  100,  NULL, 1, NULL);
        xTaskCreate(vPrinterTask, "Printer", 1000, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Sends an incrementing counter into the queue once per second. */
void vSenderTask(void *pvParameters)
{
    uint32_t ulValue = 0;

    while(1)
    {
        xQueueSend(xQueue, &ulValue, portMAX_DELAY);
        ulValue++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Owns the serial port: prints every value it receives from the queue. */
void vPrinterTask(void *pvParameters)
{
    uint32_t ulReceived;

    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        if(xQueueReceive(xQueue, &ulReceived, portMAX_DELAY) == pdPASS)
        {
            printf("Received: %lu\r\n", (unsigned long)ulReceived);
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        }
    }
}