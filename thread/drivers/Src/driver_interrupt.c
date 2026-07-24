#include "driver_interrupt.h"

/**************************************************************************
 * @fn				- interrupt_Config
 *
 * @brief			- this function enables or disables the given interrupt
 * 					  in the NVIC
 *
 * @param[in]		- IRQ number, see @IRQ_NUMBERS
 * @param[in]		- ENABLE or DISABLE macros
 *
 * @return			- none
 *
 * @Note			- none
 *
 */
void interrupt_Config(uint8_t IRQNumber, uint8_t EnorDi)
{
    // reference: CORTEX M4 user guide
	if(EnorDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			// program ISER0 register
			*NVIC_ISER0 |= (1 << IRQNumber);

		}else if(IRQNumber > 31 && IRQNumber < 64) // 32 to 63
		{
			// program ISER1 register
			*NVIC_ISER1 |= (1 << (IRQNumber % 32));

		}else if(IRQNumber >= 64 && IRQNumber < 96) // 64 to 95
		{
			// program ISER2 register
			*NVIC_ISER2 |= (1 << (IRQNumber % 64));

		}
	}else
	{
		if(IRQNumber <= 31)
		{
			// program ICER0 register
			*NVIC_ICER0 |= (1 << IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber < 64)
		{
			// program ICER1 register
			*NVIC_ICER1 |= (1 << (IRQNumber % 32));
		}else if(IRQNumber >= 64 && IRQNumber < 96)
		{
			// program ICER2 register
			*NVIC_ICER2 |= (1 << (IRQNumber % 64));
		}
	}
}

/**************************************************************************
 * @fn				- interrupt_SetPriority
 *
 * @brief			- this function sets the NVIC priority of the given
 * 					  interrupt
 *
 * @param[in]		- IRQ number, see @IRQ_NUMBERS
 * @param[in]		- priority level, see @IRQ_PRIORITY
 *
 * @return			- none
 *
 * @Note			- the priority must be set before the interrupt is
 * 					  enabled; every interrupt starts at priority 0
 *
 */
void interrupt_SetPriority(uint8_t IRQNumber, uint8_t IRQPriority)
{
	// each IPR register holds 4 interrupts, one byte each
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	// only the upper NO_PR_BITS_IMPLEMENTED bits of the byte are implemented
	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);

	*(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFF << (8 * iprx_section));
	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}