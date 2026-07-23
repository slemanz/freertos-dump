/*
 * Example: working with queue sets.
 * A queue set lets one task wait on several queues at once, without polling
 * each of them in turn. xQueueSelectFromSet() blocks until ANY member queue
 * has data, and returns the handle of the one that did. You then read from
 * that queue as usual.
 *
 * Two senders write to two separate queues at different rates. A single
 * receiver waits on the set and toggles the LED matching whichever queue
 * delivered.
 *
 * Requires configUSE_QUEUE_SETS set to 1 in FreeRTOSConfig.h.
 */
#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vRedSenderTask(void *pvParameters);
void vGreenSenderTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

static QueueHandle_t xRedQueue      = NULL;
static QueueHandle_t xGreenQueue    = NULL;
static QueueHandle_t xQueueSet      = NULL;

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

    while(1)
    {

    }
}

void vRedSenderTask(void *pvParameters)
{

}

void vGreenSenderTask(void *pvParameters)
{

}

void vReceiverTask(void *pvParameters)
{
    
}