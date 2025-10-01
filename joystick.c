#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "include/ADC.h"
#include "include/UART.h"
#include "include/bit_macros.h"
#include "include/joystick.h"

#define DEADZONE 30   
#define CALIBRATION_VALUE 4   
#define AVG_SAMPLES 2 // to average out noise

Joystick joystick;

static inline int16_t percent_axis(uint8_t val, uint8_t zero, uint8_t min, uint8_t max);
static inline Direction dir_from_xy(int16_t x_perc, int16_t y_perc);

static inline void adc_read_avg(uint8_t samples, uint8_t *out_x, uint8_t *out_y) {
    if (samples == 0) { 
        *out_x = 0;
        *out_y = 0;
        return;
    }
    uint16_t sx = 0, sy = 0;
    for (uint8_t i = 0; i < samples; i++) {
        uint8_t *d = adc_read();   // d[0] = x, d[1] = y
        sx += d[0];
        sy += d[1];
    }
    *out_x = (uint8_t)(sx / samples);
    *out_y = (uint8_t)(sy / samples);
}


//for some reasons these two functions need to use printf_P to work properly
void print_joystick(void) { 
	printf_P(PSTR("X: %d (%d%%) | Y: %d (%d%%) | Dir: %S\r\n"), 
        joystick.x_val, joystick.x_val_perc, joystick.y_val, joystick.y_val_perc,
        (joystick.dir == UP) ? PSTR("UP") : (joystick.dir == DOWN) ? PSTR("DOWN") : (joystick.dir == LEFT) ? PSTR("LEFT") : (joystick.dir == RIGHT) ? PSTR("RIGHT") : PSTR("NEUTRAL"));
	
} 

void print_zeros(void) { 
	printf_P(PSTR("X0: (%d) | Y0: (%d)\r\n"), 
        joystick.x_zero, joystick.y_zero); 
} 

void calibrate(void) {
    uint8_t x, y;
    adc_read_avg(CALIBRATION_VALUE, &x, &y);

    joystick.x_zero = x;
    joystick.y_zero = y;
}


static inline int16_t percent_axis(uint8_t val, uint8_t zero, uint8_t min, uint8_t max) {
    if (val >= zero) {
        uint16_t num = (uint16_t)(val - zero) * 100u;
        uint16_t den = (uint16_t)(max - zero);
        return den ? (int16_t)(num / den) : 0;
    } else {
        uint16_t num = (uint16_t)(zero - val) * 100u;
        uint16_t den = (uint16_t)(zero - min);
        return den ? -(int16_t)(num / den) : 0;
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

// Update joystick position and direction
void update_position(void){
    uint8_t x, y;
    adc_read_avg(AVG_SAMPLES, &x, &y);

	joystick.x_val = x;
    joystick.y_val = y;
    joystick.x_val_perc  = percent_axis(x, joystick.x_zero, 115, 255); //min and max x from testing
    joystick.y_val_perc  = percent_axis(y, joystick.y_zero, 115, 255); //min and max y from testing

    joystick.dir = dir_from_xy(joystick.x_val_perc, joystick.y_val_perc);

	printf_P(PSTR("updated position\r\n"));
	
}



