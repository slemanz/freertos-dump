/*
 * Example: receiving from the UART by polling.
 * The simplest way to read a byte is to sit in a loop asking the peripheral
 * "is there a byte yet?" over and over. UART_GetFlagStatus(UART2, UART_FLAG_RXNE)
 * is the question; reading UART_read_byte(UART2) both takes the byte and
 * clears the flag.
 *
 * The catch is that this task never blocks: it holds the CPU spinning even
 * when no data arrives. Here the polling task runs at a higher priority than a
 * heartbeat task, so the heartbeat LED FREEZES -- the busy loop starves it.
 * That is the waste interrupts exist to remove (see uart_interrupt.c).
 *
 * UART2 is already configured for RX by config_app() (UART_MODE_TXRX).
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_uart.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_RED             GPIO_PIN_NO_3   /* heartbeat */
#define LED_GREEN           GPIO_PIN_NO_5   /* one toggle per received byte */

void vPollingRxTask(void *pvParameters);
void vHeartbeatTask(void *pvParameters);

int main(void)
{
    config_app();

    led_init(GPIOB, LED_RED);
    led_init(GPIOB, LED_GREEN);

    /* Poller at the higher priority: it will starve the heartbeat. */
    xTaskCreate(vPollingRxTask, "Rx",        200, NULL, 2, NULL);
    xTaskCreate(vHeartbeatTask, "Heartbeat", 200, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
}

/*
 * Busy-waits on RXNE. Reading the data register returns the byte and clears
 * the flag in one step. Echoes the byte and toggles an LED.
 */
void vPollingRxTask(void *pvParameters)
{
    uint8_t ucByte;

    while(1)
    {
        /* Spin until a byte has arrived, this is the wasteful part. */
        while (UART_GetFlagStatus(UART2, UART_FLAG_RXNE) == FLAG_RESET);
        
        ucByte = UART_read_byte(UART2);


        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        UART_write(UART2, &ucByte, 1); /* Echo */
    }
}

/* Would blink at 2 Hz -- but the poller above never yields, so it is starved. */
void vHeartbeatTask(void *pvParameters)
{
    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}