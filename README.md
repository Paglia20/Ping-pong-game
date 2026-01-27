# Ping Pong Game – Distributed Embedded Control System

This repository contains the implementation of a **distributed embedded control system** developed as the term project for Embedded Software for the Internet of Things

The goal of the project is to design, implement, and integrate hardware and software components to realize a fully functional electromechanical ping-pong game.

VIDEO YT: https://youtu.be/EY5c8nqkt5o

PRESENTAZIONE: https://docs.google.com/presentation/d/1GyrgBDQB6DLBOCSSR9prwrPOCyiLYu961ucq15aZPro/edit?usp=sharing


## Project Overview

The system is built around two cooperating embedded nodes that communicate over a **CAN bus**.  




## System Architecture

### Node 1 – User Interface & Game Logic
- **Microcontroller**: ATmega162 (AVR)
- **Main responsibilities**:
  - Read user inputs (joystick, buttons, touch sensors)
  - Manage game logic and score
  - Drive the OLED display
  - Send control commands to Node 2 via CAN
- **Key peripherals**:
  - External SRAM via parallel memory interface
  - ADC (MAX156) for analog inputs
  - OLED display via SPI
  - CAN controller (MCP2515) via SPI
  - RS-232 interface for debugging


![System overview](images/node1.jpeg)


### Node 2 – Actuation & Control
- **Microcontroller**: ATSAM3X8E (ARM Cortex-M3, Arduino Due)
- **Main responsibilities**:
  - Servo control for paddle positioning
  - DC motor control with encoder feedback
  - Solenoid control for ball shooting
  - IR sensor processing for goal detection
- **Key peripherals**:
  - CAN transceiver (MCP2562)
  - Motor driver (A3959)
  - PWM module for servo and motor control
  - ADC for IR sensor input


![System overview](images/fullarchitecture.jpeg)


---

## Communication

- **Protocol**: CAN (Controller Area Network)
- **Purpose**:
  - Transmit joystick positions and commands from Node 1 to Node 2
  - Transmit feedback and game events from Node 2 to Node 1
- **Design approach**:
  - Layered CAN driver
  - Hardware-independent message abstraction


## Build and Flashing

### Node 1 – ATmega162
- **Toolchain**: avr-gcc, avr-libc, avrdude
- **Flashing command**:
```
make flash
```

### Node 2 – ATSAM3X8E
- **Toolchain**: gcc-arm-none-eabi
- **Debugging**: OpenOCD
- **Flashing command**:
```
make flash
```


##  PID controller:
The motor control system in Node 2 is implemented as a closed-loop control system, where the motor position measured by the encoder is continuously compared to a reference value received from Node 1. The goal is to minimize the error between the desired and the actual position.
At each control step:
The encoder provides the current motor position.
The position error is computed.
A control law updates the motor command accordingly.
This control loop is executed periodically inside a timer interrupt (TC0_Handler), ensuring a fixed and deterministic control period.
The system uses a discrete time integration approach, based on the Euler method, to update the motor state over time. In practice, the continuous motor dynamics are approximated by updating the control output using small time steps and integral error accumulation.



## Development Notes

- Node 1 operates at **5V logic**, Node 2 at **3.3V logic**  
  → Level shifting is required.
- External memory, SPI, and CAN buses were verified using oscilloscope measurements.

| Component | Node | Purpose / Function | Reference Manual |
|----------|------|--------------------|------------------|
| ATmega162 | Node 1 | Main controller for game logic, user input, display control, ADC handling, and CAN communication | ATmega162 datasheet |
| ATSAM3X8E (Arduino Due) | Node 2 | Motor control, PWM generation, encoder decoding, solenoid control, closed-loop control | SAM3X datasheet |
| ATmega16U2 | Node 2 | USB-to-serial interface for programming and debugging | Arduino Due documentation |
| MCP2515 | Node 1 | Implements CAN 2.0B protocol, message handling via SPI | MCP2515 datasheet |
| MCP2551 | Node 1 & 2 | Converts logic-level CAN signals to differential CAN bus signals | MCP2551 datasheet |
| MAX233 | Node 1 | Enables serial communication with a PC for debugging | MAX233 datasheet |
| MAX156 (8/4-channel) | Node 1 | Converts analog joystick and touch sensor signals to digital values | MAX156 datasheet |
| SRAM | Node 1 | Stores external data such as display buffers | SRAM datasheet |
| SSD1309 OLED | Node 1 | Drives the 128×64 OLED display | SSD1309 datasheet |
| OLED 128×64 | Node 1 | Visual output for game state and information | OLED module manual |
| Joystick | Node 1 | Analog directional user input | Joystick module |
| Buttons / Touch sensors | Node 1 | Additional user interaction inputs | I/O board documentation |
| A3959 | Node 2 | Drives DC motor using PWM and direction control | A3959 datasheet |
| DC Motor | Node 2 | Moves the racket | Motor datasheet |
| Quadrature Encoder | Node 2 | Provides motor position and direction feedback | SAM3X TC |
| Servo Motor | Node 2 | Controls mechanical inclination using PWM | RC servo standard |
| Solenoid | Node 2 | Shoots the ping-pong ball | Solenoid datasheet |
| IR Sensor + Op-Amp | Node 2 | Detects goal events via beam interruption | Sensor documentation |
| Voltage Regulator | Node 1 | Generates stable 5 V supply | Regulator datasheet |



