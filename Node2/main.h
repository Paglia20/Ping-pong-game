#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

#include "uart.h"
#include "can_controller.h"
#include "../include/joystick.h"
#include "interrupt.h"
#include "time.h"
#include "encoder.h"




extern const char* direction_str[];

extern volatile uint32_t ball_count;
extern volatile uint32_t prev_count;


const char* print_dir(uint8_t val);

Direction decode_dir(uint8_t val);

void qdec_tc2_init();
int32_t qdec_tc2_get_position(void);


#define BAUD 250000

