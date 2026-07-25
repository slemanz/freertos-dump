/*
 * Example: giving a binary semaphore from an interrupt.
 * This is the "deferred interrupt handling" pattern, and the single most
 * useful thing a binary semaphore does.
 *
 * The ISR must be as short as possible, and it is not allowed to block. So
 * it does nothing but give a semaphore; the task waiting on that semaphore
 * does the real work at task priority, where blocking is legal.
 *
 * Only FreeRTOS APIs whose name ends in FromISR may be called from an ISR:
 * xSemaphoreGive() is forbidden here, xSemaphoreGiveFromISR() is correct.
 *
 * xHigherPriorityTaskWoken is set to pdTRUE if the give unblocked a task
 * more important than the one that was interrupted. Handing it to
 * portYIELD_FROM_ISR() makes the context switch happen when the interrupt
 * returns, instead of waiting for the next tick.
 *
 * IMPORTANT: FreeRTOS only allows FromISR calls from interrupts whose
 * priority is numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
 * (5 here). After reset every interrupt is at priority 0 (the highest),
 * which would trip configASSERT and hang. We lower it by hand below.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LED_GREEN           GPIO_PIN_NO_5
#define BUTTON_PIN          GPIO_PIN_NO_0

void vButtonHandlerTask(void *pvParameters);

static SemaphoreHandle_t xBinarySemaphore = NULL;

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    xBinarySemaphore = xSemaphoreCreateBinary();

    if(xBinarySemaphore != NULL)
    {
        /* Button on PA0 tied to GND: pull-up, interrupt on the falling edge. */
        button_init(GPIOA, BUTTON_PIN, GPIO_MODE_IT_FT);

        /* Must be >= 5, or any FromISR call trips configASSERT. */
        interrupt_SetPriority(IRQ_NO_EXTI0, 6);
        interrupt_Config(IRQ_NO_EXTI0, ENABLE);

        xTaskCreate(vButtonHandlerTask, "Button Handler", 200, NULL, 2, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/*
 * The whole ISR: clear the hardware flag, hand the key to the task, ask for
 * a context switch on the way out. No printf, no delay, no blocking.
 */
void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Clear the EXTI pending bit or this interrupt re-fires forever. */
    EXTI->PR |= (1 << BUTTON_PIN);

    xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* The deferred half of the interrupt: here blocking IS allowed. */
void vButtonHandlerTask(void *pvParameters)
{
    uint32_t ulPresses = 0;

    led_init(GPIOB, LED_GREEN);

    while(1)
    {
        if(xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            ulPresses++;
            GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
            printf("Button pressed: %lu\r\n", (unsigned long)ulPresses);

            /*
             * Pretend the work takes a while. Presses that arrive during
             * this window are LOST: a binary semaphore does not count.
             * No debounce here: switch bounce can make one press count twice.
             */
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}