/*
 * Example: changing a task priority at runtime.
 * FreeRTOS lets you read a task's priority with uxTaskPriorityGet() and
 * change it while the program is running with vTaskPrioritySet().
 * To reference another task you need its handle (TaskHandle_t), which is
 * saved (6th argument of xTaskCreate) when the task is created.
 *
 * Here a "controller" task periodically flips a "worker" task's priority
 * between a LOW and a HIGH level, showing that priorities are not fixed
 * and can be adjusted at any moment.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_CONTROLLER      GPIO_PIN_NO_3
#define LED_WORKER          GPIO_PIN_NO_5

/*
 * Priorities: the HIGHER the number, the HIGHER the priority.
 * When raised to HIGH the worker sits ABOVE the controller; when set to
 * LOW it sits below it.
 */
#define PRIORITY_CONTROLLER     2
#define PRIORITY_WORKER_LOW     1
#define PRIORITY_WORKER_HIGH    3

void vControllerTask(void *pvParameters);
void vWorkerTask(void *pvParameters);

/*
 * Handle to the worker task. The controller needs it to address the worker
 * in uxTaskPriorityGet() / vTaskPrioritySet().
 */
static TaskHandle_t xWorkerHandle = NULL;

/*
 * Profiler: counter incremented by the worker. Inspect it with a debugger
 * to see the worker keeps running as its priority changes.
 */
typedef uint32_t TaskProfiler;
TaskProfiler WorkerTaskProfiler;

static const uint8_t controller_led = LED_CONTROLLER;
static const uint8_t worker_led     = LED_WORKER;

static void led_init(GPIO_RegDef_t *pGPIOx, uint8_t pin)
{
    GPIO_PinConfig_t led;
    led.pGPIOx = pGPIOx;
    led.GPIO_PinNumber = pin;
    led.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinSpeed = GPIO_SPEED_FAST;
    led.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    led.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    led.GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN;
    GPIO_Init(&led);
}

int main(void)
{
    config_app();

    /*
     * Create the worker first and SAVE its handle (6th argument), so the
     * controller can change its priority later.
     */
    xTaskCreate(vWorkerTask,
                "Worker Task",
                100,
                (void *)&worker_led,
                PRIORITY_WORKER_LOW,
                &xWorkerHandle);

    xTaskCreate(vControllerTask,
                "Controller Task",
                100,
                (void *)&controller_led,
                PRIORITY_CONTROLLER,
                NULL);

    vTaskStartScheduler();

    while(1)
    {
    }
}

/*
 * Controller task.
 * Every 2 seconds it reads the worker's current priority and flips it to
 * the other level, demonstrating vTaskPrioritySet() at runtime.
 */
void vControllerTask(void *pvParameters)
{
    uint8_t pin = *(const uint8_t *)pvParameters;

    led_init(GPIOB, pin);

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, pin);

        /* Read the worker's current priority. */
        UBaseType_t uxWorkerPriority = uxTaskPriorityGet(xWorkerHandle);

        /* Flip it between the LOW and HIGH levels. */
        if(uxWorkerPriority == PRIORITY_WORKER_LOW)
        {
            vTaskPrioritySet(xWorkerHandle, PRIORITY_WORKER_HIGH);
        }
        else
        {
            vTaskPrioritySet(xWorkerHandle, PRIORITY_WORKER_LOW);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/*
 * Worker task.
 * Just blinks its LED. Its priority is changed from the outside by the
 * controller; the worker itself does nothing special.
 */
void vWorkerTask(void *pvParameters)
{
    uint8_t pin = *(const uint8_t *)pvParameters;

    led_init(GPIOB, pin);

    while(1)
    {
        WorkerTaskProfiler++;
        GPIO_ToggleOutputPin(GPIOB, pin);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
