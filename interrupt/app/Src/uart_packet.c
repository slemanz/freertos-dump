/*
 * Example: receiving an entire data packet.
 * A byte-at-a-time interrupt is fine, but usually we care about whole
 * messages, not single characters. Here the ISR accumulates incoming bytes
 * into a line buffer and, when it sees an end-of-line marker (CR or LF), copies
 * the finished line into a Packet_t and posts it to a task through a queue.
 * The task then handles one complete packet at a time.
 *
 * Sending the packet by value through the queue avoids sharing the buffer
 * between the ISR and the task, so there is no race to reason about.
 *
 * Type a line in any serial terminal and press Enter; either CR or LF ends the
 * packet, so CR, LF and CRLF line endings all work.
 */
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_GREEN           GPIO_PIN_NO_5
#define PACKET_LEN          64

typedef struct
{
    char data[PACKET_LEN];
}Packet_t;

static QueueHandle_t xPacketQueue = NULL;

void vPacketTask(void *pvParameters);

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_GREEN);

    xPacketQueue = xQueueCreate(4, sizeof(Packet_t));

    if(xPacketQueue != NULL)
    {
        UART_InterruptConfig(UART2, UART_INTERRUPT_RXNEIE, ENABLE);
        interrupt_SetPriority(IRQ_NO_UART2, 6);
        interrupt_Config(IRQ_NO_UART2, ENABLE);

        xTaskCreate(vPacketTask, "Packet", 300, NULL, 2, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Handles one whole packet per iteration. */
void vPacketTask(void *pvParameters)
{
    Packet_t packet;

    while(1)
    {
        if(xQueueReceive(xPacketQueue, &packet, portMAX_DELAY) == pdPASS)
        {
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
            printf("got packet: %s\r\n", packet.data);
        }
    }
}

/*
 * The ISR assembles the line. It keeps a private buffer and index across calls
 * (static). On an end-of-line byte (CR or LF) (or if the buffer is about to
 * overflow) it terminates the string, ships it as a Packet_t and resets for
 * the next line. Empty lines are skipped, so a CRLF ending does not produce a
 * second, empty packet.
 */
void USART2_IRQHandler(void)
{
    static char cLine[PACKET_LEN];
    static uint8_t ucIndex = 0;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    Packet_t packet;
    uint8_t ucByte;

    if(UART_GetFlagStatus(UART2, UART_FLAG_RXNE) == FLAG_SET)
    {
        ucByte = UART_read_byte(UART2);

        if((ucByte == '\n') || (ucByte == '\r') || (ucIndex == (PACKET_LEN - 1)))
        {
            /* Skip and empty line: this also swallows the \n of a \r\n pair */
            if(ucIndex > 0)
            {
                cLine[ucIndex] = '\0';
                memcpy(packet.data, cLine, PACKET_LEN);
                xQueueSendFromISR(xPacketQueue, &packet, &xHigherPriorityTaskWoken);
                ucIndex = 0;
            }
        }else
        {
            cLine[ucIndex] = (char)ucByte;
            ucIndex++;
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}