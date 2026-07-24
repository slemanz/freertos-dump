#ifndef INC_DRIVER_INTERRUPT_H_
#define INC_DRIVER_INTERRUPT_H_

#include "stm32f411xx.h"

/* IRQ numbers and priority levels come from stm32f411xx.h */

/********************************************************************************************
 *                              APIs supported by this driver                               *
 *                  for more information check the function definitions                     *
 ********************************************************************************************/

void interrupt_Config(uint8_t IRQNumber, uint8_t EnorDi);

/*
 * Every interrupt sits at priority 0 (the most urgent) after reset, which is a
 * problem under FreeRTOS: an interrupt may only call a *FromISR* API if its
 * priority is numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
 * otherwise configASSERT trips and the firmware hangs. So any interrupt that
 * talks to the kernel must be given a priority before it is enabled.
 */
void interrupt_SetPriority(uint8_t IRQNumber, uint8_t IRQPriority);


#endif /* INC_DRIVER_INTERRUPT_H_ */
