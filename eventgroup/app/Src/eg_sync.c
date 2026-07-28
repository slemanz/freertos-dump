/*
 * Example: synchronizing tasks with xEventGroupSync (a rendezvous / barrier).
 * xEventGroupSync(group, uxBitsToSet, uxBitsToWaitFor, ticks) does two things
 * atomically: it sets this task's own bit, then blocks until ALL of the
 * uxBitsToWaitFor bits are set. When the last task arrives, every task unblocks
 * together and the sync bits are cleared automatically.
 *
 * Three tasks reach the barrier at different times. The "reached" lines print
 * staggered; the "passed" lines print together, because none proceeds until all
 * three have arrived.
 */

#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define BIT_A               (1 << 0)
#define BIT_B               (1 << 1)
#define BIT_C               (1 << 2)
#define ALL_BITS            (BIT_A | BIT_B | BIT_C)

typedef struct
{
    EventBits_t bit;
    const char *name;
    uint8_t led;
    uint32_t ms;
}Party_t;

static const Party_t party[3];

static const Party_t party[3] =
{
    { BIT_A, "A", GPIO_PIN_NO_3, 500  },
    { BIT_B, "B", GPIO_PIN_NO_4, 900  },
    { BIT_C, "C", GPIO_PIN_NO_5, 1400 },
};

static EventGroupHandle_t xEventGroup = NULL;

void vSyncTask(void *pvParameters);

int main(void)
{
    config_app();
    setvbuf(stdout, NULL, _IONBF, 0);

    led_init(GPIOB, party[0].led);
    led_init(GPIOB, party[1].led);
    led_init(GPIOB, party[2].led);

    xEventGroup = xEventGroupCreate();

    if(xEventGroup != NULL)
    {
        xTaskCreate(vSyncTask, "SyncA", 300, (void*)&party[0], 1, NULL);
        xTaskCreate(vSyncTask, "SyncB", 300, (void*)&party[1], 1, NULL);
        xTaskCreate(vSyncTask, "SyncC", 300, (void*)&party[2], 1, NULL);

        vTaskStartScheduler();
    }

    while(1)
    {

    }
}

void vSyncTask(void *pvParameters)
{
    const Party_t *p = (const Party_t *)pvParameters;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(p->ms));   /* each arrives at a different time */

        printf("%s reached the barrier, waiting\r\n", p->name);

        /* Set my bit and block until all three bits are set. */
        xEventGroupSync(xEventGroup, p->bit, ALL_BITS, portMAX_DELAY);

        printf("%s passed\r\n", p->name);
        GPIO_ToggleOutputPin(GPIOB, p->led);
    }
}