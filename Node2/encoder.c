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
    
    //motor to right
    printf("cal right \n\r");
    motor_write(0, 0, 15000); //right movement
    time_spinFor(msecs(1000));
    right_pos = qdec_tc2_get_position();


    printf("cal left \n\r");

    motor_write(0, 1, 15000); //left movement
    time_spinFor(msecs(1000));
    left_pos = qdec_tc2_get_position();

    int32_t pos = qdec_tc2_get_position();

    //center
    int32_t middle = (left_pos + right_pos) / 2;

    int32_t cur_pos = qdec_tc2_get_position();
    int32_t error = middle - cur_pos;
    //printf("error: %ld\n\r", error);


    while (abs(error) > DEAD_BEND) {
        cur_pos = qdec_tc2_get_position();
        error = middle - cur_pos;
        //printf("error: %ld\n\r", error);
        if (error < CENTER_TOL) {
            motor_write(0, 0, 8000); //right
            time_spinFor(msecs(NUDGE));
        } else if (error > -CENTER_TOL) {
            motor_write(0, 1, 8000); //left
            time_spinFor(msecs(NUDGE));
        } else {
            break;
        }
        time_spinFor(msecs(5));
    }
    
    motor_write(0, 2, 0); //stop motor

    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;
    left_pos = - middle;
    right_pos = middle;

    printf("cal done \n\r");
    printf("left: %ld, right: %ld\n\r", left_pos, right_pos);
}



void encoder_movement(int8_t dir_x, int8_t dir_y) {

    printf("Encoder movement not imple");


}


