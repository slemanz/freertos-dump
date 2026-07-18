# Project Template

This is the bare-metal STM32F411 project the rest of the repository is built on:
a minimal but complete skeleton that boots the chip, brings up the clocks and
drivers, links FreeRTOS in, and hands control to a single example task. Copy it
to start a new experiment, or read it to see how the pieces below fit together.

## Layout

- [`app/`](app) - the application itself: `main.c`, the `config` layer that
  initializes the drivers, `FreeRTOSConfig.h` where the kernel is tuned, and the
  `Makefile` that builds the image.
- [`drivers/`](drivers/README.md) - the bare-metal STM32F411 drivers the
  application runs on.
- [`linkers/`](linkers) - the linker script and startup code that place the image
  in flash and bring the C runtime up before `main`.
- [workspace files](workspaces) - editor and debugger configuration for the
  project.

## Building

The firmware is built from [`app/`](app) with its `Makefile`:

```sh
make        # compile and link the firmware image
make load   # flash the image to the board over J-Link
make clean  # remove build artifacts
```
