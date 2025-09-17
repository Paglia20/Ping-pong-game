#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
 #include <stdlib.h>

#include "include/ADC.h"
#include "include/UART.h"
#include "include/bit_macros.h"
#include "include/joystick.h"

#define DEADZONE 20      

Positions pos;

static inline int16_t percent_axis(uint8_t val, uint8_t zero);
static inline Direction dir_from_xy(int16_t x_perc, int16_t y_perc);

// Defines PD2 and PD3 as input Buttons
volatile void init_button(void) {
	clear_bit(DDRD,PD2); 
	clear_bit(DDRD,PD3);
}

/* volatile void print_joystick(void) { 
	printf("X: (%4d%%) | Y: (%4d%%) | Dir: %s\r\n", 
        pos.x_val_perc, pos.y_val_perc, 
        (pos.dir == UP) ? "UP   " : (pos.dir == DOWN) ? "DOWN " : (pos.dir == LEFT) ? "LEFT " : (pos.dir == RIGHT) ? "RIGHT" : "NEUTRAL");
} */

/* void print_zeros(void) { 
	printf("X0: (%d) | Y0: (%d)\r\n", 
        pos.x_zero, pos.y_zero); 
} */

volatile void calibrate(void) {
	uint8_t *data;
	data = adc_read();
	
	pos.x_zero = *data;
	pos.y_zero = *(data+1);
}


static inline int16_t percent_axis(uint8_t val, uint8_t zero) {
    if (val >= zero) {
        uint16_t num = (uint16_t)(val - zero) * 100u;
        uint16_t den = (uint16_t)(255u - zero);
        return den ? (int16_t)(num / den) : 0;
    } else {
        uint16_t den = (zero > 0) ? (uint16_t)(zero - 1u) : 1u;
        uint16_t num = (uint16_t)val * 100u;
        return (int16_t)(num / den) - 100;
    }
}

static inline Direction dir_from_xy(int16_t x_perc, int16_t y_perc) {
    // manual abs to avoid any signed/width surprises
    int16_t ay = (y_perc >= 0) ? y_perc : (int16_t)(-y_perc);
    if (x_perc >  DEADZONE && x_perc >= ay) return RIGHT;
    if (x_perc < -DEADZONE && -x_perc >= ay) return LEFT;
    if (y_perc >  DEADZONE) return UP;
    if (y_perc < -DEADZONE) return DOWN;
    return NEUTRAL;
}

// Update joystick position and direction che richiama:

volatile void update_position(void){
    uint8_t *data;
	data = adc_read();
    
    uint8_t x, y;
    x = *data;
    y = *(data+1);
	pos.x_val = x;
    pos.y_val = y;
    pos.x_val_perc  = percent_axis(x, pos.x_zero);
    pos.y_val_perc  = percent_axis(y, pos.y_zero);

    pos.dir = dir_from_xy(pos.x_val_perc, pos.y_val_perc);

}

