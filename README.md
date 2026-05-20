# Smart Traffic Monitoring and Control System

## Overview

This project implements an adaptive monitoring and control system for a 4-way intersection using the **STM32 NUCLEO-L476RG** board. The solution utilizes **8 IR sensors** to estimate traffic levels and controls the traffic lights for two non-conflicting flows:

- **EW**: East-West
- **NS**: North-South

The current implementation groups the directions into two traffic axes, ensuring that only one axis has a green light at any given time. The green light duration is dynamically adjusted based on the number of vehicles detected on each axis.

In addition to controlling the traffic light LEDs, the system displays the remaining time and the number of detected vehicles on an **I2C 20x4 LCD** and transmits the same information via **UART2** for serial monitoring.

## Implemented Features

- Traffic reading from 8 IR sensors connected via external interrupts
- Software debounce for vehicle counting
- FSM (Finite State Machine) for traffic light phase control
- Safety intervals for yellow and all-red phases
- Dynamic adaptation of green durations based on traffic density
- Software interlock to prevent simultaneous green lights on conflicting directions
- Real-time LCD display showing remaining time and current traffic count
- UART logging for testing and debugging

## Software Architecture

Source code structure:

- `include/defines.h` - pin definitions, time constants, and state types
- `include/setup.h` - prototypes for initialization and auxiliary interfaces
- `src/setup.c` - clock, GPIO, UART, I2C, and LCD configuration, as well as EXTI interrupt handling
- `src/main.c` - main logic, traffic light FSM, and the adaptive algorithm

### Implemented FSM

The FSM utilizes 6 states:

1. `STATE_ALL_RED_1`
2. `STATE_EW_GREEN`
3. `STATE_EW_YELLOW`
4. `STATE_ALL_RED_2`
5. `STATE_NS_GREEN`
6. `STATE_NS_YELLOW`

The sequence of transitions is:

`ALL_RED -> EW_GREEN -> EW_YELLOW -> ALL_RED -> NS_GREEN -> NS_YELLOW -> repeat`

This sequence guarantees that conflicting directions never receive a green light simultaneously.

## Adaptive Algorithm

Traffic counting is split across two axes:

- `EW_traffic_cnt` for the East and West sensors
- `NS_traffic_cnt` for the North and South sensors

At the end of the NS cycle, the `Update_Traffic_Lights()` function compares the difference between the two counters and selects one of the following timing tiers:

- `TIER_0_BASE`: 60 s EW, 60 s NS
- `TIER_1_EW_75`: 75 s EW, 45 s NS
- `TIER_2_EW_90`: 90 s EW, 30 s NS
- `TIER_1_NS_75`: 45 s EW, 75 s NS
- `TIER_2_NS_90`: 30 s EW, 90 s NS

Escalation to the 90 s tier only occurs if high traffic on that specific axis is sustained for two consecutive cycles, preventing overreactions to brief traffic spikes.

## Safety Mechanisms

- **Software interlock** in `Set_Traffic_Lights()`; if a green light is requested simultaneously for both EW and NS, the system enters an error state and flashes the yellow lights.
- **Yellow and all-red duration** is fixed at `5000 ms`.
- **Non-blocking main loop**; standard transitions are managed using `HAL_GetTick()` instead of hardcoded delays.
- **Software debounce** of `250 ms` for incoming IR sensor pulses.

## Hardware Connections

### Traffic Lights (GPIOA)

- `PA5` - EW green
- `PA6` - EW yellow
- `PA7` - EW red
- `PA8` - NS yellow
- `PA9` - NS green
- `PA10` - NS red

### IR Sensors (GPIOB, EXTI, pull-up)

- `PB5` - E sensor 1
- `PB6` - E sensor 2
- `PB7` - W sensor 1
- `PB10` - W sensor 2
- `PB11` - N sensor 1
- `PB12` - N sensor 2
- `PB13` - S sensor 1
- `PB14` - S sensor 2

### UART

- `PA2` - USART2 RX
- `PA3` - USART2 TX
- Baudrate: `115200`

### I2C LCD

- `PB8` - I2C1 SCL
- `PB9` - I2C1 SDA
- Address used in code: `0x4E` for transmission

## How the System Works

1. The IR sensors generate hardware interrupts on the falling edge.
2. The EXTI callback increments the corresponding axis counter, provided the software debounce time has elapsed.
3. The FSM runs continuously in the main loop, transitioning to the next phase when the current timeout expires.
4. At the end of a full cycle, the green light durations are recalculated based on the accumulated traffic data.
5. The LCD and UART periodically output the remaining time and current counter values.

## Notes on the Current Implementation

- The current implementation utilizes **2 main green phases** (one for the EW axis and one for the NS axis), which is well-suited for a standard intersection with synchronized opposing flows.
- Vehicle counting is based on passage detection rather than continuous queue length estimation.
- Average wait times and extended historical statistics are not yet calculated separately, though the core framework for UART logging is already fully established.

## Software Requirements and Compilation

The codebase is written in C and utilizes the **STM32 HAL** library tailored for the STM32L4 family. The project is designed for seamless integration into **PLatform.io VS Code Extension**.

In its current state, this repository contains the core application logic; the STM32 HAL Library is excluded.

## Possible Improvements

- Expanding the logic to 4 independent phases for East, West, North, and South flows.
- Calculating the average waiting time per direction.
- Implementing statistical logging over fixed time intervals.
- Calibrating separate debounce intervals tailored to each individual sensor.
- Adding a hardware watchdog and an explicit fail-safe mode.
- Creating a PC graphical user interface (GUI) for remote data collection and analytics via UART.
