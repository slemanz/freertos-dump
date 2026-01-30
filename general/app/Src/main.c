#include "config.h"

#include "driver_gpio.h"
#include "driver_systick.h"

int main(void)
{
    config_app();

    uint64_t start_time = ticks_get();

    while(1)
    {
        if((ticks_get() - start_time) >= 500)
        {
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
            start_time = ticks_get();
        }
    }
}
