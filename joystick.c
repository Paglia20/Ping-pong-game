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

#define DEADZONE 20   
#define CALIBRATION_VALUE 5   
#define AVG_SAMPLES 2

Joystick joystick;

static inline int16_t percent_axis(uint8_t val, uint8_t zero, uint8_t min, uint8_t max);
static inline Direction dir_from_xy(int16_t x_perc, int16_t y_perc);


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
    uint8_t all_data[CALIBRATION_VALUE * 2]; 

    for (int i = 0; i < CALIBRATION_VALUE; i++) {
        uint8_t *data = adc_read();  // assumes data[0] = x, data[1] = y
        all_data[i * 2]     = data[0]; // x value
        all_data[i * 2 + 1] = data[1]; // y value
        printf("Calibration N %d...\r\n", i);
    }

    uint16_t x_sum = 0;
    uint16_t y_sum = 0;

    for (int i = 0; i < CALIBRATION_VALUE; i++) {
        x_sum += all_data[i * 2];     // x values: 0, 2, 4, 6, 8
        y_sum += all_data[i * 2 + 1]; // y values: 1, 3, 5, 7, 9
    }

    joystick.x_zero = x_sum / CALIBRATION_VALUE;
    joystick.y_zero = y_sum / CALIBRATION_VALUE;
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
    uint8_t all_data[AVG_SAMPLES * 2]; 

    for (int i = 0; i < AVG_SAMPLES; i++) {
        uint8_t *data = adc_read();  // assumes data[0] = x, data[1] = y
        all_data[i * 2]     = data[0]; // x value
        all_data[i * 2 + 1] = data[1]; // y value
    }

    uint16_t x_sum = 0;
    uint16_t y_sum = 0;
    uint8_t x, y;

    for (int i = 0; i < AVG_SAMPLES; i++) {
        x_sum += all_data[i * 2];     
        y_sum += all_data[i * 2 + 1]; 
    }
    x = x_sum / AVG_SAMPLES;
    y = y_sum / AVG_SAMPLES;

	joystick.x_val = x;
    joystick.y_val = y;
    joystick.x_val_perc  = percent_axis(x, joystick.x_zero, 115, 255); //min and max x from testing
    joystick.y_val_perc  = percent_axis(y, joystick.y_zero, 115, 255); //min and max y from testing

    joystick.dir = dir_from_xy(joystick.x_val_perc, joystick.y_val_perc);

	printf_P(PSTR("updated position\r\n"));
	
}



