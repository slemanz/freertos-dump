/*
 * Example: the gatekeeper task.
 * A gatekeeper task has SOLE ownership of a resource. No other task touches
 * it directly; they all ask the gatekeeper to do the work for them, by
 * sending a request through a queue.
 *
 * Because there is exactly one accessor, mutual exclusion is structural:
 * there is no mutex to take, so there is nothing to forget to release, no
 * priority inversion, and no possible deadlock. The queue already
 * serialises everything.
 *
 * Here the gatekeeper owns the serial port. A heartbeat task, a button task
 * (PA0) and an ADC task (potentiometer on PA1) all "print" by posting a
 * string into the queue.
 */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "driver_gpio.h"
#include "driver_adc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_RED             GPIO_PIN_NO_3
#define LED_GREEN           GPIO_PIN_NO_5
#define BUTTON_PIN          GPIO_PIN_NO_0

#define MSG_LEN             48
#define QUEUE_LEN           5

/* The queue carries fixed-size character buffers, by value. */
typedef struct
{
    char cText[MSG_LEN];
}PrintMsg_t;

void vGatekeeperTask(void *pvParameters);
void vHeartbeatTask(void *pvParameters);
void vButtonTask(void *pvParameters);
void vPotTask(void *pvParameters);

static QueueHandle_t xPrintQueue = NULL;

/**
 * The ONLY way any task is allowed to print. Builds the message and hands
 * it to the gatekeeper. Never blocks forever: if the queue is full the
 * message is dropped rather than stalling the caller.
 */
static void print_request(const char *pcText)
{
    PrintMsg_t msg;

    strncpy(msg.cText, pcText, MSG_LEN - 1);
    msg.cText[MSG_LEN - 1] = '\0';

    xQueueSend(xPrintQueue, &msg, pdMS_TO_TICKS(10));
}

int main(void)
{
    config_app();

    setvbuf(stdout, NULL, _IONBF, 0);

    xPrintQueue = xQueueCreate(QUEUE_LEN, sizeof(PrintMsg_t));

    if(xPrintQueue != NULL)
    {
        led_init(GPIOB, LED_RED);
        led_init(GPIOB, LED_GREEN);

        /* Polled button: plain input, no EXTI, so no NVIC priority needed. */
        button_init(GPIOA, BUTTON_PIN, GPIO_MODE_IN);

        /* adc_pa1_init() also puts PA1 in analog mode for us. */
        adc_pa1_init();
        adc_start_conversion();

        /*
         * The gatekeeper runs at the highest priority so the serial port
         * is drained promptly and the queue does not fill up.
         */
        xTaskCreate(vGatekeeperTask, "Gatekeeper", 300, NULL, 3, NULL);
        xTaskCreate(vHeartbeatTask,  "Heartbeat",  200, NULL, 1, NULL);
        xTaskCreate(vButtonTask,     "Button",     200, NULL, 1, NULL);
        xTaskCreate(vPotTask,        "Pot",        300, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/*
 * The single owner of the serial port. This is the ONLY function in the
 * whole program that calls printf.
 */
void vGatekeeperTask(void *pvParameters)
{
    PrintMsg_t msg;

    while(1)
    {
        if(xQueueReceive(xPrintQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            printf("%s\r\n", msg.cText);
        }
    }
}

/* Proves the system is alive; prints through the gatekeeper. */
void vHeartbeatTask(void *pvParameters)
{
    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        print_request("Heartbeat");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/*
 * Polls the button. Pressed reads 0, because the pin is pulled up and the
 * button shorts it to ground.
 */
void vButtonTask(void *pvParameters)
{
    uint8_t ucLast = 1;
    uint8_t ucNow;

    while(1)
    {
        ucNow = GPIO_ReadFromInputPin(GPIOA, BUTTON_PIN);

        /* Falling edge: released -> pressed. */
        if((ucLast == 1) && (ucNow == 0))
        {
            GPIO_ToggleOutputPin(GPIOB, LED_RED);
            print_request("Button pressed");
        }

        ucLast = ucNow;

        /* Polling every 20 ms doubles as the debounce. */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* Reads the potentiometer and reports it through the gatekeeper. */
void vPotTask(void *pvParameters)
{
    char cLine[MSG_LEN];
    uint32_t ulRaw;

    while(1)
    {
        ulRaw = adc_read();

        snprintf(cLine, MSG_LEN, "pot = %lu", (unsigned long)ulRaw);
        print_request(cLine);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}