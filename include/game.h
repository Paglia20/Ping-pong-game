#define F_CPU 4915200UL // 4.9152 MHz
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "../include/UART.h"
#include "../include/SRAM.h"
#include "../include/decoder.h"
#include "../include/ADC.h"
#include "../include/joystick.h"
#include "../include/slider.h"
#include "../include/bit_macros.h"
#include "../include/SPI.h"
#include "../include/OLED.h"
#include "../include/cursor.h"
#include "../include/menu.h"
#include "../include/CAN.h"
#include "../include/MCP2515.h"
#include <avr/interrupt.h>

void start_game(void);