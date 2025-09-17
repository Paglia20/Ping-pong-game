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
#include "include/slider.h"

Slider slider;

static inline int16_t percent_axis(uint8_t val, uint8_t zero, uint8_t min, uint8_t max);


//for some reasons these two functions need to use printf_P to work properly
void print_slider(void) { 
	printf_P(PSTR("Slider X: %d (%d%%)\r\n"), 
        slider.x_val, slider.x_val_perc);
} 

void print_slider_zeros(void) { 
	printf_P(PSTR("X0: (%d)\r\n"), 
        slider.x_zero); 
} 

void calibrate_slider(void) {
	uint8_t *data = adc_read();

	slider.x_zero = data[2];
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



// Update joystick position and direction
void update_slider(void){
    uint8_t *data = adc_read();

    uint8_t x;
    x = data[2];
	slider.x_val = x;
    slider.x_val_perc  = percent_axis(x, slider.x_zero, 0, 255);

	printf_P(PSTR("updated position slider\r\n"));
	
}

