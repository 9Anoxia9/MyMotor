# MyMotor — FOC Motor Control Firmware

A hand-written **Field-Oriented Control (FOC)** motor driver firmware for the
AT32F455 MCU. Features a three-loop cascade (position / velocity / torque), a
dq-frame current loop with SVPWM, absolute-encoder PLL, open-loop V/F startup,
and automatic direction & electrical-angle calibration.

The architecture and control-loop timing are inspired by
[ODrive](https://github.com/odriverobotics/ODrive), but the code is written
from scratch.

## Features

- **Three-loop cascade** — position / velocity / torque control modes
- **FOC current loop** — dq-frame PI controller with decoupling and back-EMF feedforward
- **SVPWM** — space-vector modulation with linear-region clamping and anti-windup
- **Absolute-encoder PLL** — AS5047P (14-bit) position & velocity estimation
- **Open-loop V/F control** — for startup and calibration
- **Automatic calibration**
  - Direction calibration (encoder sign detection)
  - Electrical-angle offset calibration (d-axis alignment)
- **On-the-fly current DC-offset calibration** — removes ADC bias drift
- **Over-current protection**
- **FreeRTOS + TinyUSB** — USB CDC interface for parameter query/set and
  oscilloscope data streaming

## Hardware

| Item | Spec |
|---|---|
| MCU | AT32F455 (192 MHz) |
| Motor | 7 pole-pair PMSM/BLDC, R = 0.214 Ω, L = 54.2 µH |
| Position sensor | AS5047P 14-bit absolute magnetic encoder (SPI) |
| Bus voltage | 24 V |
| PWM | Center-aligned, 24 kHz |
| Current loop | 8 kHz (125 µs) |

## Architecture

The firmware is layered, with a clean separation between the control law,
the motor, and the axis-level state machine:

```
PhaseCtrlLaw  (interface: FOC, VFControl)
      ▲
      │  implements
      │
    Motor      — current sampling, DC-offset cal, over-current, PWM output
      ▲
      │
     Axis      — state machine: IDLE / DIR_CAL / OFFSET_CAL / OPEN_LOOP / CLOSE_LOOP
      │
   Encoder     — AS5047P SPI sampling + position/velocity PLL
   Control     — position/velocity/torque cascade
```

- **`FOC`** — the closed-loop current control law (Clarke → Park → PI + feedforward → inverse-Park → SVPWM).
- **`VFControl`** — the open-loop V/F law used before the current loop is engaged.
- **`Encoder`** — decodes the AS5047P absolute position and runs a PLL to estimate
  velocity, with configurable `direction` and `phase_offset_cpr`.
- **`Axis`** — the top-level state machine that owns calibration and the
  transition between open-loop and closed-loop control.

## Directory Structure

```
project/
├── main.cpp              # entry point, FreeRTOS task creation
├── board/                # low-level peripheral init (TIM/ADC/SPI/USB/CRM)
├── drivers/              # thin GPIO/SPI C++ wrappers
├── motorcontrol/
│   ├── foc.cpp/hpp       # FOC + VF control laws
│   ├── motor.cpp/hpp     # motor abstraction, current sampling, protection
│   ├── encoder.cpp/hpp   # AS5047P decoding + PLL
│   ├── axis.cpp/hpp      # axis state machine + calibration
│   ├── control.cpp/hpp   # control setpoints
│   ├── openloop.cpp/hpp  # open-loop V/F controller
│   ├── utils.cpp/hpp     # SVPWM, wrapping helpers
│   └── oscilloscope.cpp/hpp
├── communicate/          # USB CDC + USART backends, application protocol
└── debug/                # logging
```

## Calibration

The electrical angle must be calibrated before closing the current loop:

1. **Direction calibration** — run the motor open-loop in one direction and
   detect the sign of the encoder velocity to set `Encoder::_config.direction`.
2. **Offset calibration** — apply a fixed d-axis voltage (or current) to align
   the rotor N-pole with the α-axis, then record the encoder position as
   `Encoder::_config.phase_offset_cpr`.

After calibration, `iq` produces forward torque and the current loop tracks
`id`/`iq` setpoints.

## Current Status & Known Issues

The three-loop cascade (position → velocity → torque) runs closed-loop. Known
issues are tracked in [`debug_log.md`](debug_log.md); the main ones:

- **Encoder timing is synchronized by a deterministic SPI busy-flag wait.** The
  AS5047P SPI sample finishes ~6 µs after the control-loop ISR starts; under
  optimization, `Encoder::update()` could run first and read a two-cycle-old
  sample (250 µs lag instead of 125 µs). `Encoder::update()` now waits for the SPI
  busy flag to clear before reading — a hardware-event wait (not a magic-number
  delay), so it is deterministic across `-O0` / `-O1` / `-O2`.
- **Residual USB-communication jitter** — after fixing the encoder lag, a second,
  independent jitter source remains: USB CDC traffic. Slight at `-O1` (audible
  noise only), obvious at `-O2`. Mechanism not yet pinned (AHB bus contention vs.
  analog coupling into the ADC) — see `debug_log.md`.
- **Current-loop steady-state oscillation at high speed** (`iq` ∝ 1/speed) — still
  under investigation (suspects: back-EMF λ inaccuracy, phase margin, residual
  angle lag).
- **Planned** — split the control-loop ISR into valley-compute / peak-write to
  remove the hard-wait and raise the control rate (RPR 2 → 1 → 0).

## Acknowledgements

This project is inspired by [ODrive](https://github.com/odriverobotics/ODrive),
an excellent open-source FOC servo drive. Thanks to the ODrive team for their
great work — many design ideas here are learned from their codebase.
