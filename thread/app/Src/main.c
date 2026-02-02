#include <stdio.h>
#include "config.h"
#include "driver_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

void vBlueLedControllerTask(void *pvParameters);
void vRedLedControllerTask(void *pvParameters);
void vGreenLedControllerTask(void *pvParameters);

typedef uint32_t TaskProfiler;

TaskProfiler BlueTaskProfiler, RedTaskProfiler, GreenTaskProfiler;

int main(void)
{
    config_app();

    xTaskCreate(vBlueLedControllerTask,
                "Blue Led Controller",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vGreenLedControllerTask,
                "Green Led Controller",
                100,
                NULL,
                1,
                NULL);

    xTaskCreate(vRedLedControllerTask,
                "Red Led Controller",
                100,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();

    while(1)
    {
    }
}

void vBlueLedControllerTask(void *pvParameters)
{
    while(1)
    {
        BlueTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vRedLedControllerTask(void *pvParameters)
{
    while(1)
    {
        RedTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vGreenLedControllerTask(void *pvParameters)
{
    while(1)
    {
        GreenTaskProfiler++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}