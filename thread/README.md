# Thread Management

### Creating a task

```C
BaseType_t xTaskCreate( TaskFunction pvTaskCode,
                        const char *const pcName,
                        uint16_t usStackDepth,
                        void *pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t *pxCreatedTask );
```

- **[main example code](app/Src/main.c)**

### Creting Tasks from other Tasks

```C
void vBlueLEDControllerTask(void *pvParameters)
{
	xTaskCreate(vRedLEDControllerTask,"Red LED Controller", 100, NULL, 1, NULL);
	xTaskCreate(vGreenLEDControllerTask,"Green LED Controller", 100, NULL, 1, NULL);

	while(1)
	{
		blueTaskProfiler++;
	}

}
```

You don't need to call `vTaskStartScheduler()` again.