/*
 * Example: setting event bits from an interrupt.
 * The button on PA0 raises an EXTI interrupt; the ISR announces it by setting a
 * bit with xEventGroupSetBitsFromISR, and a task blocked on that bit does the
 * work. Setting bits can unblock several tasks -- too much for an ISR -- so the
 * kernel DEFERS the actual set to the timer daemon; that is why configUSE_TIMERS
 * must be on (it is here).
 *
 * As always with FromISR: the interrupt must be given a priority numerically
 * >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5), set BEFORE it is enabled.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define LED_GREEN           GPIO_PIN_NO_5
#define BUTTON_PIN          GPIO_PIN_NO_0

#define BIT_BUTTON          (1 << 0)

static EventGroupHandle_t xEventGroup = NULL;

void vHandlerTask(void *pvParameters);

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, LED_GREEN);

    /* Falling-edge EXTI on PA0 (button to ground, internal pull-up). */
    button_init(GPIOA, BUTTON_PIN, GPIO_MODE_IT_FT);
    interrupt_SetPriority(IRQ_NO_EXTI0, 6);   /* >= 5, before enabling */
    interrupt_Config(IRQ_NO_EXTI0, ENABLE);
    
    xEventGroup = xEventGroupCreate();

    if(xEventGroup != NULL)
    {
        xTaskCreate(vHandlerTask, "Handler", 300, NULL, 2, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Blocks on the button bit; wakes when the ISR sets it. */
void vHandlerTask(void *pvParameters)
{
    while(1)
    {
        xEventGroupWaitBits(xEventGroup, BIT_BUTTON, pdTRUE, pdTRUE, portMAX_DELAY);

        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        printf("button event\r\n");
    }
}

/*
 * The ISR: clear the EXTI pending bit, set the event bit (deferred to the
 * daemon), and yield if that made a higher-priority task ready.
 */
void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    EXTI->PR |= (1 << BUTTON_PIN);   /* clear the pending bit */

    xEventGroupSetBitsFromISR(xEventGroup, BIT_BUTTON, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}