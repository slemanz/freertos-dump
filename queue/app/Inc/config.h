#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include <stdint.h>

#include "driver_gpio.h"

void config_app(void);
void config_interface(void);
void config_core(void);

/*
 * Pin helpers shared by every example, so none of them has to fill a
 * GPIO_PinConfig_t by hand.
 */
void led_init(GPIO_RegDef_t *pGPIOx, uint8_t pin);
void button_init(GPIO_RegDef_t *pGPIOx, uint8_t pin, uint8_t mode);

#endif /* INC_CONFIG_H_ */
