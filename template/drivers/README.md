# Drivers

These are the bare-metal drivers the FreeRTOS examples run on, written from
scratch for the STM32F411 with no vendor HAL in between. Each one talks to the
peripheral registers directly, which keeps the code small and makes exactly what
reaches the hardware visible. The goal is not a feature-complete HAL but a clear,
minimal foundation the RTOS layer can sit on.

Every driver is split the same way: the header in [`Inc/`](Inc) declares the
public interface and the configuration macros, while the source in [`Src/`](Src)
holds the implementation, with each function documented at its definition.

## The Drivers

- **Clock** — set up the system clock and enable the peripheral clocks.
  [`Inc/driver_clock.h`](Inc/driver_clock.h) ·
  [`Src/driver_clock.c`](Src/driver_clock.c)
- **GPIO** — configure and drive general-purpose I/O pins.
  [`Inc/driver_gpio.h`](Inc/driver_gpio.h) ·
  [`Src/driver_gpio.c`](Src/driver_gpio.c)
- **UART** — serial transmit and receive with configurable baud, word length,
  parity, and stop bits.
  [`Inc/driver_uart.h`](Inc/driver_uart.h) ·
  [`Src/driver_uart.c`](Src/driver_uart.c)
- **SysTick** — the core system tick used for timekeeping and as the RTOS time
  base.
  [`Inc/driver_systick.h`](Inc/driver_systick.h) ·
  [`Src/driver_systick.c`](Src/driver_systick.c)
- **Timer** — the general-purpose timer peripherals, including output-compare and
  PWM generation.
  [`Inc/timer.h`](Inc/timer.h) ·
  [`Src/timer.c`](Src/timer.c)
- **ADC** — analog-to-digital conversion.
  [`Inc/driver_adc.h`](Inc/driver_adc.h) ·
  [`Src/driver_adc.c`](Src/driver_adc.c)
- **Interrupt** — enable, prioritize, and dispatch NVIC interrupts.
  [`Inc/driver_interrupt.h`](Inc/driver_interrupt.h) ·
  [`Src/driver_interrupt.c`](Src/driver_interrupt.c)
- **Flash** — on-chip flash access and the wait-state configuration the clock
  setup depends on.
  [`Inc/driver_flash.h`](Inc/driver_flash.h) ·
  [`Src/driver_flash.c`](Src/driver_flash.c)
- **FPU** — enable the hardware floating-point unit.
  [`Inc/driver_fpu.h`](Inc/driver_fpu.h) ·
  [`Src/driver_fpu.c`](Src/driver_fpu.c)
