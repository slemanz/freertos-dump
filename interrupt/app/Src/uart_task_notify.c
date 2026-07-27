/*
 * Example: deferring to a task with a direct task notification.
 * A task notification is the lightest way for an ISR to unblock a task: it
 * needs no separate queue or semaphore object, just the task's handle, and it
 * is faster and uses less RAM. The notification even carries a 32-bit value,
 * so here it carries the received byte itself.
 *
 *   ISR:  xTaskNotifyFromISR(handle, byte, eSetValueWithOverwrite, &woken)
 *   task: xTaskNotifyWait(0, 0xFFFFFFFF, &value, portMAX_DELAY)
 *
 * Trade-off vs. a queue (uart_interrupt.c): a notification does not queue. If
 * two bytes arrive before the task runs, the overwrite keeps only the last.
 * Notifications are ideal for signalling; a queue is what you want when every
 * byte must be buffered.
 *
 * Task notifications are enabled by default (configUSE_TASK_NOTIFICATIONS).
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_GREEN           GPIO_PIN_NO_5

/* The ISR needs the handle of the task it notifies. */
static TaskHandle_t xHandlerTask = NULL;

void vHandlerTask(void *pvParameters);

int main(void)
{
    config_app();

    config_app();

    led_init(GPIOB, LED_GREEN);

    UART_InterruptConfig(UART2, UART_INTERRUPT_RXNEIE, ENABLE);
    interrupt_SetPriority(IRQ_NO_UART2, 6);
    interrupt_Config(IRQ_NO_UART2, ENABLE);

    /* Save the handle so the ISR can reach this task. */
    xTaskCreate(vHandlerTask, "Handler", 200, NULL, 2, &xHandlerTask);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vHandlerTask(void *pvParameters)
{
    uint32_t ulValue;
    uint8_t ucByte;

    while(1)
    {
        if(xTaskNotifyWait(0, 0xFFFFFFFF, &ulValue, portMAX_DELAY) == pdTRUE)
        {
            ucByte = (uint8_t)ulValue;

            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
            UART_write(UART2, &ucByte, 1); /* echo */
        }
    }
}

void USART2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t ucByte;

    if(UART_GetFlagStatus(UART2, UART_FLAG_RXNE) == FLAG_SET)
    {
        ucByte = UART_read_byte(UART2);
        xTaskNotifyFromISR(xHandlerTask, (uint32_t)ucByte,
                           eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}