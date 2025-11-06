#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

#include "uart.h"
#include "can_controller.h"
#include "../include/joystick.h"
#include "interrupt.h"
#include "time.h"
#include "encoder.h"



#define PIN_TIOA6   (1u << 25)  // PC25
#define PIN_TIOB6   (1u << 26)  // PC26


extern const char* direction_str[];

extern volatile uint32_t ball_count;
extern volatile uint32_t prev_count;

static inline void servo_write(uint32_t ch, uint16_t us);
static inline void motor_write(uint32_t ch, uint8_t dir);


const char* print_dir(uint8_t val);

Direction decode_dir(uint8_t val);

void qdec_tc2_init();
int32_t qdec_tc2_get_position(void);


#define BAUD 250000

