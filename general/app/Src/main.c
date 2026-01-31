#include "config.h"

#include "driver_gpio.h"
#include "driver_systick.h"

int main(void)
{
    config_app();


    while(1)
    {
        //GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
    }
}
