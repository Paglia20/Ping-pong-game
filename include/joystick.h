#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include <stdbool.h>

// Joystick direction enum
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NEUTRAL
} Direction;

// Joystick and slider position structure
typedef struct {
    volatile int16_t x_zero;
    volatile int16_t y_zero;
    volatile int16_t x_val;
    volatile int16_t y_val;
    volatile int16_t x_val_perc;
    volatile int16_t y_val_perc;
    volatile Direction dir;
} Positions;

// External global variable defined in joystick.c
extern Positions pos;

void init_button(void);

void print_joystick(void);

void print_zeros(void);

void calibrate(void);

void update_position(void);

#endif // JOYSTICK_H