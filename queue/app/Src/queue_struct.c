/*
 * Example: sending more complex data with queues.
 * A queue item does not have to be a scalar. As long as every item is the
 * same fixed size, it can be a struct. Here we pass a struct by value: a
 * copy of the whole struct is placed into the queue.
 *
 * Two sender tasks put a struct carrying their own id and a counter into a
 * single queue. One receiver reads whatever arrives and toggles a different
 * LED depending on which sender it came from.
 *
 * (For large payloads you would send a POINTER to the data instead of a copy,
 * to save memory and time - but then you must guarantee the data outlives the
 * trip through the queue.)
 */

 #include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

/* Identifies which sender produced the item */
typedef enum
{
    SENDER_RED = 0,
    SENDER_GREEN
}SenderId_t;

/* The fixed-size item carried by the queue. */
typedef struct
{
    SenderId_t id;
    uint32_t value;
}Message_t;

void vSenderTask(void *pvParameters);
void vReceiverTask(void *pvParameters);

static QueueHandle_t xQueue = NULL;

/* Seeds passed toeach sender task by pointer; they never change. */
static const Message_t xRedSeed   = {SENDER_RED,   0 };
static const Message_t xGreenSeed = {SENDER_GREEN, 0 };

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

    /* The queue now holds whole Message_t structs. */
    xQueue = xQueueCreate(5, sizeof(Message_t));

    if(xQueue != NULL)
    {

    }

    while(1)
    {

    }
}

void vSenderTask(void *pvParameters)
{

}

void vReceiverTask(void *pvParameters)
{
    
}