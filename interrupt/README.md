# Interrupt Management

An interrupt is the hardware calling you. Instead of your code repeatedly asking
a peripheral whether something has happened, the peripheral stops the CPU the
instant it does and runs a dedicated function, the *interrupt service routine*
(ISR). That is wonderful for responsiveness, but it sits awkwardly next to an
RTOS: an ISR runs outside the scheduler, at a priority the scheduler does not
control, and while it runs the task that was interrupted is frozen. This chapter
is about living with that tension safely: keeping ISRs short, handing their work
to tasks, and using only the parts of the kernel API that are legal from an
interrupt. The runnable examples live under [`app/Src/`](app/Src/), one file per
concept, and are built on the project skeleton described under
[Template](../template/README.md).

Everything here drives UART2, which `config_app()` already configures for both
transmit and receive at 115200 8N1. The USART2 interrupt is `IRQ_NO_UART2` (38)
and its handler is `USART2_IRQHandler`.

## Polling versus Interrupts

The baseline for comparison is *polling*: a loop that keeps asking the peripheral
"is there a byte yet?" `UART_GetFlagStatus(UART2, UART_FLAG_RXNE)` is that
question, and `UART_read_byte(UART2)` both returns the byte and clears the flag.
It works, but the loop never blocks, so it burns the CPU spinning even when no
data is arriving, and at a high priority it will starve every other task. This is
the same busy-wait problem as in the thread chapter, and it is exactly what
interrupts remove: the CPU does other useful work, or nothing at all, until the
byte actually arrives. See `uart_polling.c`.