#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include <stdbool.h>


#define JOY_B_PORT    PORTD
#define JOY_B_DDR     DDRD
#define JOY_B_PIN     PD3

// Joystick direction enum
typedef enum Direction{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NEUTRAL
} Direction;

// Joystick and slider position structure
typedef struct Joystick{
    volatile int16_t x_zero;
    volatile int16_t y_zero;
    volatile int16_t x_val;
    volatile int16_t y_val;
    volatile int16_t x_val_perc;
    volatile int16_t y_val_perc;
    volatile Direction dir;
    volatile bool button;
} Joystick;

// External global variable defined in joystick.c
extern Joystick joystick;

void print_joystick(void);

void print_zeros(void);

void calibrate(void);

void update_joystick(void);

#endif // JOYSTICK_H