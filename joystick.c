#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

#include "include/ADC.h"
#include "include/UART.h"
#include "include/bit_macros.h"
#include "include/joystick.h"

// Joystick directions
enum directions {
	UP, DOWN, LEFT, RIGHT, NEUTRAL
};

typedef struct {											
	int16_t x_zero;
	int16_t y_zero;
	volatile int16_t x_val;
	volatile int16_t y_val;
	volatile int16_t x_val_perc;
	volatile int16_t y_val_perc;
	volatile enum directions dir;
	volatile uint8_t slider1_val;
	volatile uint8_t slider2_val;
} Positions;

Positions pos;


// Defines PD3 and PD4 as input Buttons
void init_button(void) {
	clearBit(DDRD,PD3); 
	clearBit(DDRD,PD4);
}

void print_joystick(void) { 
	printf("X: %4d (%4d%%) | Y: %4d (%4d%%) | Dir: %s | S1: %3d | S2: %3d\r\n", 
        pos.x_val_perc, pos.y_val_perc, 
        (pos.dir == UP) ? "UP   " : (pos.dir == DOWN) ? "DOWN " : (pos.dir == LEFT) ? "LEFT " : (pos.dir == RIGHT) ? "RIGHT" : "NEUTRAL",
        pos.slider1_val, pos.slider2_val);
}

void calibrate(void) {
	uint8_t *data;
	data = readADC();
	
	pos.x_zero = *data;
	pos.y_zero = *(data+1);
}

// Update joystick position and direction che richiama:

// Percentage calculation

// Direction calculation


