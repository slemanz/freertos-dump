/*
 * Example: centralised deferred handling with xTimerPendFunctionCallFromISR.
 * Sometimes you do not want a dedicated task per interrupt. This API lets an
 * ISR ask the timer service task (the daemon) to run an ordinary function
 * later, on its behalf, passing it one pointer and one 32-bit value:
 *
 *   xTimerPendFunctionCallFromISR(function, pvParam1, ulParam2, &woken)
 *
 * The function then runs in the daemon's task context, where it is free to do
 * things an ISR must not. Here the ISR pends a function that echoes the byte
 * and toggles an LED, so there is no handler task in this program at all.
 *
 * Requires INCLUDE_xTimerPendFunctionCall (1 here) and configUSE_TIMERS (1).
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED_GREEN           GPIO_PIN_NO_5

/* Runs in the timer daemon task, deferred from the ISR. */
void vDeferredHandler(void *pvParameter1, uint32_t ulParameter2)
{
    uint8_t ucByte = (uint8_t)ulParameter2;

    GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
    UART_write(UART2, &ucByte, 1);   /* echo */
}

int main(void)
{
    config_app();

    led_init(GPIOB, LED_GREEN);

    UART_InterruptConfig(UART2, UART_INTERRUPT_RXNEIE, ENABLE);
    interrupt_SetPriority(IRQ_NO_UART2, 6);
    interrupt_Config(IRQ_NO_UART2, ENABLE);

    /* No handler task: the daemon does the deferred work. */
    vTaskStartScheduler();

    while(1)
    {

    }
}

void USART2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t ucByte;

    if(UART_GetFlagStatus(UART2, UART_FLAG_RXNE) == FLAG_SET)
    {
        ucByte = UART_read_byte(UART2);
        xTimerPendFunctionCallFromISR(vDeferredHandler, NULL, 
                                      (uint32_t)ucByte, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}