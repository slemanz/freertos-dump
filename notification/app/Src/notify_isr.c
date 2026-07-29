/*
 * Example: notifying a task from an interrupt.
 * A notification can be sent from an ISR with vTaskNotifyGiveFromISR (the
 * FromISR twin of xTaskNotifyGive). This is the one direction that works:
 * ISR -> task. A task can NOT notify an ISR.
 *
 * The button on PA0 raises an EXTI interrupt; the ISR notifies a task, which
 * does the real work deferred at task priority.
 *
 * The interrupt must have an NVIC priority numerically >= 5
 * (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY), set BEFORE it is enabled.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_GREEN           GPIO_PIN_NO_5
#define BUTTON_PIN          GPIO_PIN_NO_0

static TaskHandle_t xHandlerTask = NULL;

void vHandlerTask(void *pvParameters);

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_GREEN);

    button_init(GPIOA, BUTTON_PIN, GPIO_MODE_IT_FT);
    interrupt_SetPriority(IRQ_NO_EXTI0, 6);   /* >= 5, before enabling */
    interrupt_Config(IRQ_NO_EXTI0, ENABLE);

    xTaskCreate(vHandlerTask, "Handler", 300, NULL, 2, &xHandlerTask);

    vTaskStartScheduler();

    while(1)
    {

    }
}

void vHandlerTask(void *pvParameters)
{
    while(1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        printf("button event\r\n");
    }
}

void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    EXTI->PR |= (1 << BUTTON_PIN);   /* clear the pending bit */

    vTaskNotifyGiveFromISR(xHandlerTask, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}