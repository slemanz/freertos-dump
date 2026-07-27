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

## The Interrupt-Safe API

FreeRTOS splits its API in two. Ordinary functions such as `xQueueSend` may only
be called from a task; calling them from an ISR corrupts the kernel. For use
inside an interrupt there is a parallel set of functions whose names end in
`FromISR` — `xQueueSendFromISR`, `xSemaphoreGiveFromISR`, `xTaskNotifyFromISR`,
and so on. An ISR may call *only* these.

The `FromISR` functions take one extra argument, a pointer to a
`BaseType_t xHigherPriorityTaskWoken`, which the call sets to `pdTRUE` if it
unblocked a task more important than the one the interrupt suspended. You pass
that flag to `portYIELD_FROM_ISR` at the end of the ISR, and if it is set the
scheduler switches to the woken task as the interrupt returns, instead of waiting
for the next tick:

```c
void USART2_IRQHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t ucByte;

    if( UART_GetFlagStatus( UART2, UART_FLAG_RXNE ) == FLAG_SET )
    {
        ucByte = UART_read_byte( UART2 );
        xQueueSendFromISR( xRxQueue, &ucByte, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```

## Interrupt Priorities and the configASSERT Trap

There is a hardware rule that catches everyone. FreeRTOS only tolerates `FromISR`
calls from interrupts whose priority is numerically greater than or equal to
`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`, which is `5` in this project.
After reset every interrupt sits at priority `0`, the most urgent, so an interrupt
that talks to the kernel without having its priority lowered first will trip
`configASSERT` and hang the moment it calls in.

The cure is to set the priority before enabling the interrupt. The driver exposes
both steps, and the order matters:

```c
UART_InterruptConfig( UART2, UART_INTERRUPT_RXNEIE, ENABLE );
interrupt_SetPriority( IRQ_NO_UART2, 6 );   /* >= 5: lower the urgency first */
interrupt_Config( IRQ_NO_UART2, ENABLE );   /* then let it fire */
```

## Deferred Interrupt Handling

The single most important pattern in this chapter is *deferred interrupt
handling*: the ISR does the bare minimum and hands the real work to a task. The
ISR reads the byte, passes it on, and returns in a few microseconds; a task,
blocked and costing nothing until then, wakes up and does the processing at task
priority, where it is free to block, print, or take a mutex. A queue is the
natural carrier because it both signals the task and delivers the data, and it
buffers, so a short burst of bytes is not lost if the task is briefly busy. See
`uart_interrupt.c`, where the heartbeat LED keeps blinking because nothing spins.

## Assembling a Packet

Real protocols are messages, not lone characters, and the ISR is a good place to
assemble one. It keeps a private buffer across calls and appends each byte until
it sees an end-of-line marker, a carriage return or a newline, at which point it
ships the finished line to a task and starts the next one. Passing the completed
packet *by value* through the queue means the ISR and the task never share the
buffer, so there is no race to reason about. See `uart_packet.c`; typing a line
in any serial terminal and pressing Enter delivers a packet, whatever line ending
the terminal sends.

## Task Notifications

When all you need is to unblock one specific task, a full queue or semaphore is
more than the job requires. A *task notification* is a lighter, faster channel
built into every task: the ISR calls `xTaskNotifyFromISR` with the target task's
handle, and the task waits with `xTaskNotifyWait`. No separate kernel object is
allocated. A notification even carries a 32-bit value, so it can deliver the
received byte directly. The trade-off is that it does not queue: with
`eSetValueWithOverwrite`, a second notification arriving before the task runs
overwrites the first. Notifications are ideal for signalling; reach for a queue
when every item must be kept. Task notifications are on by default
(`configUSE_TASK_NOTIFICATIONS`). See `uart_task_notify.c`.

## Centralised Deferral

Sometimes you would rather not create a dedicated handler task at all. The
`xTimerPendFunctionCall` family lets an ISR ask the timer service task, the daemon
from the previous chapter, to run an ordinary function on its behalf a moment
later. From an interrupt you use `xTimerPendFunctionCallFromISR`, passing a
function together with one pointer and one 32-bit argument; the function then runs
in the daemon's context, free to do what an ISR cannot. It is a tidy way to funnel
many interrupt sources through one deferred-work task. It needs
`INCLUDE_xTimerPendFunctionCall` and `configUSE_TIMERS`, both `1` here. See
`deferred_pend.c`.