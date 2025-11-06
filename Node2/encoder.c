#include "encoder.h"

volatile int32_t right_pos = 0;
volatile int32_t left_pos = 0;

//The datasheet’s Figure 36-15 shows A=TIOA0, B=TIOB0 per TC block; for TC2 those become TIOA6, TIOB6 ￼
void qdec_tc2_init(void) {

    PMC->PMC_PCER0 |= (1u << ID_PIOC);

    //Route PC25/PC26(/PC29) to peripheral B (TC2)
    PIOC->PIO_PDR = PIN_TIOA6 | PIN_TIOB6;   // hand pins to peripheral
    PIOC->PIO_ABSR |= PIN_TIOA6 | PIN_TIOB6; // select B function

    //Enable TC2 peripheral clock 
    PMC->PMC_PCER1 |= (1u << (ID_TC6 - 32));


    // Configure TC2 in Quadrature Decoder mode
    //    – QDEN   : enable quadrature decoding
    //    – POSEN  : position enabled (counter on ch.0)
    //    – EDGPHA : A leads B
    //    – MAXFILT: small digital filter to reject bounce/noise on A/B
    TC2->TC_BMR =
        TC_BMR_QDEN      |
        TC_BMR_POSEN     |
        TC_BMR_EDGPHA    |
        TC_BMR_MAXFILT(3);   // adjust 0..63 as needed (filter = (MAXFILT+1) * t_periph)


    // In QDEC clock source is XC0, Start channels 0 
    TC2->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_XC0;  

    // Enable + trigger (reset to zero on SWTRG)
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
}

int32_t qdec_tc2_get_position(void) {
    return (int32_t)TC2->TC_CHANNEL[0].TC_CV;
}





void encode_init(void) {
    
    qdec_tc2_init();
    int32_t pos = qdec_tc2_get_position();
    int32_t prev_pos = pos - 1;

    //put motor to right;
    while (pos > prev_pos) {
        prev_pos = pos;
        motor_write(0, 0); //right movement
        pos = qdec_tc2_get_position();
    }

    right_pos = pos;
    prev_pos = pos + 1;

    //put motor to left;
    while (pos < prev_pos) {
        prev_pos = pos;
        motor_write(0, 1); //left movement
        pos = qdec_tc2_get_position();
    }

    left_pos = pos;

    //center
    middle = (left_pos + right_pos) / 2;
    while (pos < middle || pos > middle) {
        pos = qdec_tc2_get_position();
        if (pos < middle) {
            motor_write(0, 0);
        } else if (pos > middle) {
            motor_write(0, 1);
        }
    }
    motor_write(0, 2); //stop motor

    qdec_tc2_init(); //reinitialize to reset position to 0
    left_pos = - middle;
    right_pos = middle;
}



void encoder_movement(int8_t dir_x, int8_t dir_y) {

    printf("Encoder movement not imple");


}


