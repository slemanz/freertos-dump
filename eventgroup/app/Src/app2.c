/*
 * Example app2: blink the red LED and print over the serial port.
 * Two independent tasks running at the same priority (round-robin):
 *   - vRedLedTask toggles the red LED (GPIOB, pin 3) every 500 ms;
 *   - vSerialTask prints "Hello World" over UART2 every 5 seconds.
 *
 * printf is retargeted to UART2 in config.c (__io_putchar).
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3

void vRedLedTask(void *pvParameters);
void vSerialTask(void *pvParameters);

int main(void)
{
    config_app();

    /* Disable stdout buffering so each printf is sent out immediately. */
    setvbuf(stdout, NULL, _IONBF, 0);

    xTaskCreate(vRedLedTask,
                "Red Led",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vSerialTask,
                "Serial",
                100,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();

    while(1)
    {
    }
}

/* Toggles the red LED every 500 ms. */
void vRedLedTask(void *pvParameters)
{
    led_init(GPIOB, LED_RED);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* Prints "Hello World" over the serial port every 5 seconds. */
void vSerialTask(void *pvParameters)
{
    while(1)
    {
        printf("Hello World\r\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
