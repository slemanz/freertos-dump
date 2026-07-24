/*
 * Example: working with binary semaphores.
 * A semaphore is a signal, or a key: it carries no data, it only says
 * "something happened" or "you may proceed".
 *
 * A binary semaphore holds a single flag: taken or available.
 *
 *   xSemaphoreCreateBinary()          -> create it (starts EMPTY!)
 *   xSemaphoreTake(sem, ticksToWait)  -> wait for / grab the key
 *   xSemaphoreGive(sem)               -> hand the key over
 *
 * Because it is created empty, the taker blocks until the first give.
 * And because it is a single flag, giving twice before it is taken is the
 * same as giving once: the second give is simply lost. That is exactly the
 * limitation counting semaphores exist to fix (see sem_counting.c).
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5

void vGiverTask(void *pvParameters);
void vTakerTask(void *pvParameters);

static SemaphoreHandle_t xBinarySemaphore = NULL;

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

void vGiverTask(void *pvParameters)
{

}

void vTakerTask(void *pvParameters)
{
    
}