#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

#include "uart.h"
#include "can.h"
#include "../include/joystick.h"
#include "interrupt.h"
#include "time.h"


#define PIN_TIOA6   (1u << 25)  // PC25
#define PIN_TIOB6   (1u << 26)  // PC26
#define NUDGE   8   
#define DEAD_BAND   4


static inline void servo_write(uint32_t ch, uint16_t us)
{
    if (us != PWM->PWM_CH_NUM[ch].PWM_CDTYUPD) {
        if (us < 900)  us = 900;     
        if (us > 2100) us = 2100;   
        PWM->PWM_CH_NUM[ch].PWM_CDTYUPD = us;
    }

    //printf("servo: %d\n\r", PWM->PWM_CH_NUM[ch].PWM_CDTY);
}

static inline void motor_write(uint32_t ch, uint8_t dir, uint32_t speed)
{   
    if (dir == 0) {
        // right = HIGH = set
        PWM->PWM_CH_NUM[ch].PWM_CDTYUPD = speed;             

        PIOC -> PIO_SODR = (1u << 23);
        
    } else if (dir == 1) {
        PWM->PWM_CH_NUM[ch].PWM_CDTYUPD = speed;             

        // left = LOW = clear
        PIOC -> PIO_CODR = (1u << 23);
    } else {
        // center = disable both
        PWM->PWM_CH_NUM[ch].PWM_CDTYUPD = 0;             
    }
}



void encode_init();
void set_point(int8_t dir_x);
void control_timer_init(void);
void update_motor(void);
