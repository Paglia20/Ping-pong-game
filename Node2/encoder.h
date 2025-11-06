#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

#include "uart.h"
#include "can_controller.h"
#include "../include/joystick.h"
#include "interrupt.h"
#include "time.h"
#include "main.h"

void encode_init();
void encoder_movement(int8_t dir_x, int8_t dir_y);