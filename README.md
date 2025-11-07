# Term Project – Embedded and Industrial Computer Systems Design (TTK4155)

This repository contains the implementation for the **TTK4155 Term Project** at **NTNU**.

The project focuses on designing and developing an embedded control system, using two microcontrollers: an Atmega162 and an Arduino Due

## System Description

The system integrates two microcontrollers that communicate to coordinate the behavior of the ping pong game.  
This demonstrates distributed embedded control in a real-time environment.

## Setup and Communication

To list connected peripherals:
```bash
sudo dmesg | grep tty

picocom -b 115200 /dev/ttyACM0

picocom -b 9600 --databits 8 --parity n --stopbits 2 --flow n /dev/ttys0
