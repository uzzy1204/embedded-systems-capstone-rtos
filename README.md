# Embedded Systems Capstone — Custom RTOS + Energy Monitor/Control

> STM32-based system with a **student-built RTOS**, 1 kHz sensing, wireless telemetry, and actuator control.

## Repo layout
- `firmware/` — target firmware for MCU
  - `src/` — app sources (`main.c`, init)
  - `include/` — shared headers
  - `rtos/` — our scheduler, task, IPC, timers
  - `drivers/` — HAL/wrapper for sensors, relays
  - `boards/STM32F4xx/` — board-specific (clocks, pins)
- `host-sim/` — tiny POSIX build to sanity-check the scheduler on your laptop
- `docs/` — architecture, timing budgets, test plans
- `tools/` — scripts for formatting, flashing, logging
- `.github/workflows/` — CI for host-sim build

