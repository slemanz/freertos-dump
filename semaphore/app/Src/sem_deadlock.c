/*
 * Example: deadlock (the "deadly embrace").
 * Two tasks, two mutexes, taken in OPPOSITE orders:
 *
 *   Task A: take mutex 1 ... then take mutex 2
 *   Task B: take mutex 2 ... then take mutex 1
 *
 * If A gets mutex 1 and B gets mutex 2 before either gets the second one,
 * each is holding exactly what the other needs. Neither can proceed, and
 * neither ever releases. Both are stuck forever.
 *
 * The vTaskDelay between the two takes is only there to make the race
 * reliable - a real deadlock needs no help, it just happens rarely enough
 * to reach production before you notice.
 *
 * NOTE: the serial output interleaves even with the fix - printf is not
 * guarded by any mutex here. Watch for liveness, not for clean text.
 *
 * TWO CLASSIC CURES:
 *   1. Lock ORDERING: every task always takes the mutexes in the same
 *      global order. That is what FIX_DEADLOCK does here.
 *   2. Take with a TIMEOUT instead of portMAX_DELAY, and on failure give
 *      back whatever you already hold and retry.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define FIX_DEADLOCK        1

#define LED_RED             GPIO_PIN_NO_3
#define LED_YELLOW          GPIO_PIN_NO_4
#define LED_GREEN           GPIO_PIN_NO_5

void vTaskA(void *pvParameters);
void vTaskB(void *pvParameters);
void vHeartbeatTask(void *pvParameters);

static SemaphoreHandle_t xMutex1 = NULL;
static SemaphoreHandle_t xMutex2 = NULL;

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

    xMutex1 = xSemaphoreCreateMutex();
    xMutex2 = xSemaphoreCreateMutex();

    if(xMutex1 != NULL && xMutex2 != NULL)
    {
        led_init(GPIOB, LED_RED);
        led_init(GPIOB, LED_YELLOW);
        led_init(GPIOB, LED_GREEN);

        xTaskCreate(vTaskA,         "Task A",    300, NULL, 1, NULL);
        xTaskCreate(vTaskB,         "Task B",    300, NULL, 1, NULL);
        xTaskCreate(vHeartbeatTask, "Heartbeat", 200, NULL, 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

/* Takes mutex 1, then mutex 2. */
void vTaskA(void *pvParameters)
{
    while(1)
    {
        printf("A: taking mutex 1\r\n");
        xSemaphoreTake(xMutex1, portMAX_DELAY);

        /* Long enough for B to grab mutex 2 in the meantime */
        vTaskDelay(pdMS_TO_TICKS(50));

        printf("A: taking mutex 2\r\n");
        xSemaphoreTake(xMutex2, portMAX_DELAY); /* <-- blocks forever */

        GPIO_ToggleOutputPin(GPIOB, LED_RED);
        printf("A: got both\r\n");

        /* Release in reverse order of acquisition. */
        xSemaphoreGive(xMutex2);
        xSemaphoreGive(xMutex1);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* Takes them in the opposite order - unless the fix is enabled. */
void vTaskB(void *pvParameters)
{
    while(1)
    {
#if(FIX_DEADLOCK == 1)
        /* Same global order as A: 1 then 2. No cycle, no deadlock. */
        printf("B: taking mutex 1\r\n");
        xSemaphoreTake(xMutex1, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(50));

        printf("B: taking mutex 2\r\n");
        xSemaphoreTake(xMutex2, portMAX_DELAY);
#else
        /* Opposite order: this is the bug */
        printf("B: taking mutex 2\r\n");
        xSemaphoreTake(xMutex2, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(50));

        printf("B: taking mutex 1\r\n");
        xSemaphoreTake(xMutex1, portMAX_DELAY); /* <-- blocks forever */
#endif
        GPIO_ToggleOutputPin(GPIOB, LED_YELLOW);
        printf("B: got both\r\n");

        xSemaphoreGive(xMutex2);
        xSemaphoreGive(xMutex1);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/*
 * Touches no mutex at all, so it keeps blinking after A and B are dead.
 * That is the signature of a deadlock: the system looks alive, but two
 * specific tasks stopped forever.
 */
void vHeartbeatTask(void *pvParameters)
{
    while(1)
    {
        GPIO_ToggleOutputPin(GPIOB, LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}