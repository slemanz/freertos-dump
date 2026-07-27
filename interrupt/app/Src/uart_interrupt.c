/*
 * Example: interacting with an interrupt service routine (deferred handling).
 * Instead of polling, we let the UART raise an interrupt when a byte arrives.
 * The golden rule of interrupt handling under an RTOS: do as little as
 * possible in the ISR and defer the real work to a task.
 *
 * The RXNE interrupt fires, the ISR reads the byte and hands it to a task
 * through a queue with xQueueSendFromISR, then returns. The handler task,
 * blocked on the queue, wakes up and does the work at task priority. Because
 * nothing busy-waits, the heartbeat LED now blinks steadily.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

static QueueHandle_t xRxQueue = NULL;

void vHandlerTask(void *pvParameters);
void vHeartbeatTask(void *pvParameters);

int main(void)
{
    config_app();
    
    led_init(GPIOB, LED_RED);
    led_init(GPIOB, LED_GREEN);

    xRxQueue = xQueueCreate(16, sizeof(uint8_t));

    if(xRxQueue != NULL)
    {
        /* Enable the RXNE interrupt in the UART, then wire it into the NVIC. */
        UART_InterruptConfig(UART2, UART_INTERRUPT_RXNEIE, ENABLE);
        interrupt_SetPriority(IRQ_NO_UART2, 6);   /* >= 5, so FromISR is legal */
        interrupt_Config(IRQ_NO_UART2, ENABLE);

        xTaskCreate(vHandlerTask,   "Handler",   200, NULL, 2, NULL);
        xTaskCreate(vHeartbeatTask, "Heartbeat", 200, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/*
 * The deferred handler. Blocks on the queue with no CPU cost until the ISR
 * posts a byte, then echoes it and toggles the LED.
 */
void vHandlerTask(void *pvParameters)
{
    uint8_t ucByte;

    while(1)
    {
        if(xQueueReceive(xRxQueue, &ucByte, portMAX_DELAY))
        {
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
            UART_write(UART2, &ucByte, 1); /* echo */
        }
    }

}

void vHeartbeatTask(void *pvParameters)
{
    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

/*
 * The ISR: minimal. Read the byte (which clears RXNE), pass it to the task,
 * and request a context switch if that unblocked a higher-priority task.
 */
void USART2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTasWoken = pdFALSE;
    uint8_t ucByte;

    if(UART_GetFlagStatus(UART2, UART_FLAG_RXNE) == FLAG_SET)
    {
        ucByte = UART_read_byte(UART2);
        xQueueSendFromISR(xRxQueue, &ucByte, &xHigherPriorityTasWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTasWoken);
}