#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

#include "uart.h"
#include "can_controller.h"
#include "../include/joystick.h"
#include "interrupt.h"


extern volatile uint32_t ball_count = 0;
extern volatile uint32_t prev_count = 0;

#define BAUD 250000

