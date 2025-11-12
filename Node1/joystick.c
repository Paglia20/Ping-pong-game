#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "../include/CAN.h"
#include "../include/ADC.h"
#include "../include/UART.h"
#include "../include/joystick.h"

#define DEADZONE 20   
#define CALIBRATION_VALUE 10   
#define AVG_SAMPLES 10 // to average out noise


//min and max x from testing
//min and max y from testing
#define X_MIN 68
#define X_MAX 248
#define Y_MIN 65
#define Y_MAX 249

Joystick joystick;

static inline int8_t percent_axis(uint8_t val, uint8_t zero, uint8_t min, uint8_t max);
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


//uses uart_puts_p
void print_joystick(void) {
    if (!DEBUG) return;
    char buf[8];

    uart_puts_P(PSTR("X: "));
    itoa(joystick.x_val, buf, 10);
    uart_puts(buf);

    uart_puts_P(PSTR(" ("));
    itoa(joystick.x_val_perc, buf, 10);
    uart_puts(buf);
    uart_puts_P(PSTR("%) | Y: "));

    itoa(joystick.y_val, buf, 10);
    uart_puts(buf);

    uart_puts_P(PSTR(" ("));
    itoa(joystick.y_val_perc, buf, 10);
    uart_puts(buf);
    uart_puts_P(PSTR("%) | Dir: "));

    switch (joystick.dir) {
        case UP:       uart_puts_P(PSTR("UP")); break;
        case DOWN:     uart_puts_P(PSTR("DOWN")); break;
        case LEFT:     uart_puts_P(PSTR("LEFT")); break;
        case RIGHT:    uart_puts_P(PSTR("RIGHT")); break;
        default:       uart_puts_P(PSTR("NEUTRAL")); break;
    }

    uart_puts_P(PSTR("\r\n"));
}

void print_zeros(void) {
    if (!DEBUG) return;

    char buf[8];
    uart_puts_P(PSTR(")\r\n"));

    uart_puts_P(PSTR("X0: ("));
    itoa(joystick.x_zero, buf, 10);
    uart_puts(buf);

    uart_puts_P(PSTR(") | Y0: ("));
    itoa(joystick.y_zero, buf, 10);
    uart_puts(buf);
    uart_puts_P(PSTR(")\r\n"));
}

void calibrate(void) {
    //printf("Calibrating joystick\r\n");
    uint8_t x, y;
    adc_read_avg(CALIBRATION_VALUE, &x, &y);

    // JOY_B input
    JOY_B_DDR &= ~(1<<JOY_B_PIN);

    joystick.x_zero = x;
    joystick.y_zero = y;
    print_zeros();

}


static inline int8_t percent_axis(uint8_t val, uint8_t zero, uint8_t min, uint8_t max) {
    if (val >= zero) {
        uint16_t num = (uint16_t)(val - zero) * 100u;
        uint16_t den = (uint16_t)(max - zero);
        return den ? (int8_t)(num / den) : 0;
    } else {
        uint16_t num = (uint16_t)(zero - val) * 100u;
        uint16_t den = (uint16_t)(zero - min);
        return den ? -(int8_t)(num / den) : 0;
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
void update_joystick(void){
    uint8_t x, y;
    adc_read_avg(AVG_SAMPLES, &x, &y);

    joystick.button = !(PINB & (1 << JOY_B_PIN));  // PINB

	joystick.x_val = x;
    joystick.y_val = y;
    joystick.x_val_perc  = percent_axis(x, joystick.x_zero, X_MIN, X_MAX); 
    joystick.y_val_perc  = percent_axis(y, joystick.y_zero, Y_MIN, Y_MAX); 

    joystick.dir = dir_from_xy(joystick.x_val_perc, joystick.y_val_perc);

	//printf_P(PSTR("updated position X: %d Y: %d\r\n"), joystick.x_val, joystick.y_val);
	
}
