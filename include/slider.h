#ifndef SLIDER_H
#define SLIDER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    volatile int16_t x_zero;
    volatile int16_t x_val;
    volatile int8_t x_val_perc;
} Slider;

extern Slider slider;

void print_slider(void);
void print_slider_zeros(void);
void calibrate_slider(void);
void update_slider(void);

#endif // SLIDER_H