#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

#include "uart.h"
#include "can_controller.h"
#include "../include/joystick.h"
#include "interrupt.h"
#include "time.h"


#define PIN_TIOA6   (1u << 25)  // PC25
#define PIN_TIOB6   (1u << 26)  // PC26
#define CAL_VALUE   10    //number of steps to move during calibration


static inline void servo_write(uint32_t ch, uint16_t us)
{
    if (us != PWM->PWM_CH_NUM[ch].PWM_CDTYUPD) {
        if (us < 900)  us = 900;     
        if (us > 2100) us = 2100;   
        PWM->PWM_CH_NUM[ch].PWM_CDTYUPD = us;
    }

    //printf("servo: %d\n\r", PWM->PWM_CH_NUM[ch].PWM_CDTY);
}

static inline void motor_write(uint32_t ch, uint8_t dir)
{
    if (dir == 0) {
        // right = HIGH = set
        PWM->PWM_CH_NUM[ch].PWM_CDTY = 13000;             

        PIOC -> PIO_SODR = (1u << 23);
        
    } else if (dir == 1) {
        PWM->PWM_CH_NUM[ch].PWM_CDTY = 13000;             

        // left = LOW = clear
        PIOC -> PIO_CODR = (1u << 23);
    } else {
        // center = disable both
        PWM->PWM_CH_NUM[ch].PWM_CDTY = 0;             
    }
}



void encode_init();
void encoder_movement(int8_t dir_x, int8_t dir_y);