# Thread Management

**Creating a task:**

```C
BaseType_t xTaskCreate( TaskFunction pvTaskCode,
                        const char *const pcName,
                        uint16_t usStackDepth,
                        void *pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t *pxCreatedTask );
```