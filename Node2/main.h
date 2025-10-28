#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

#include "uart.h"
#include "can_controller.h"
#include "../include/joystick.h"
#include "interrupt.h"

extern const char* direction_str[];

extern volatile uint32_t ball_count;
extern volatile uint32_t prev_count;

#define BAUD 250000

