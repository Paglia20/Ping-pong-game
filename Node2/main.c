/*
 * Remember to update the Makefile with the (relative) path to the uart.c file.
 * This starter code will not compile until the UART file has been included in the Makefile. 
 * If you get somewhat cryptic errors referencing functions such as _sbrk, 
 * _close_r, _write_r, _fstat etc, you have most likely not done that correctly.

 * If you get errors such as "arm-none-eabi-gcc: no such file", you may need to reinstall the arm gcc packages using
 * apt or your favorite package manager.
 */

#include "main.h"

#define PIN_TIOA6   (1u << 25)  // PC25
#define PIN_TIOB6   (1u << 26)  // PC26

volatile uint32_t ball_count;
volatile uint32_t prev_count;

const char* direction_str[] = {
    "UP", "DOWN", "LEFT", "RIGHT", "NEUTRAL"
};

static inline void servo_write(uint32_t ch, uint16_t us)
{
    if (us < 900)  us = 900;     
    if (us > 2100) us = 2100;   
    PWM->PWM_CH_NUM[ch].PWM_CDTYUPD = us;

    //printf("servo: %d\n\r", PWM->PWM_CH_NUM[ch].PWM_CDTY);
}

static inline void motor_write(uint32_t ch, uint8_t dir)
{
    if (dir == 0) {
        // right = HIGH = set
        PWM->PWM_CH_NUM[ch].PWM_CDTY = 10000;             

        PIOC -> PIO_SODR = (1u << 23);
        
    } else if (dir == 1) {
        PWM->PWM_CH_NUM[ch].PWM_CDTY = 10000;             

        // left = LOW = clear
        PIOC -> PIO_CODR = (1u << 23);
    } else {
        // center = disable both
        PWM->PWM_CH_NUM[ch].PWM_CDTY = 0;             
    }
}


int main()
{   
    SystemInit();

    WDT->WDT_MR = WDT_MR_WDDIS; //Disable Watchdog Timer

    /* !! set high servo pin
       PMC ->PMC_PCER0 = (1 << ID_PIOB); 
       PIOB ->PIO_PER |= PIO_PB12;
       PIOB ->PIO_OER |= PIO_PB12; 
       PIOB ->PIO_SODR |= PIO_PB12; */

    /* !! Enable clock for test should be 84MHz

       PMC->PMC_PCER0 = (1u << ID_PIOA);

       // Hand PA1 to peripheral and select the right mux for PCK0 on PA1
       PIOA->PIO_PDR   = PIO_PDR_P1;       // disable PIO, enable peripheral control
       PIOA->PIO_ABSR |= PIO_ABSR_P1;    // select Peripheral B for PCK0

       // Start PCK0 = MCK/2 (42 MHz with MCK=84 MHz)
       PMC->PMC_PCK[0] = PMC_PCK_CSS_MCK | PMC_PCK_PRES_CLK_2;
       PMC->PMC_SCER   = PMC_SCER_PCK0;
       while ((PMC->PMC_SR & PMC_SR_PCKRDY0) == 0) {} */



    // --- PWM SERVO channel 1 del PWM ---
    PMC->PMC_PCER0 |= (1u << ID_PIOB);    //  PIOB manipulation
    PMC->PMC_PCER1 |= (1 << (ID_PWM - 32));   //  PWM clock

    // choosing PB13
    PIOB->PIO_PDR   = PIO_PDR_P13;              // disable GPIO control
    PIOB->PIO_ABSR |= PIO_ABSR_P13;             // select Peripheral B (PWMH1)

    //Disable channel during setup
    PWM->PWM_DIS = PWM_DIS_CHID1;

    // Set Channel 1 to use CLKA
    PWM->PWM_CLK = PWM_CLK_PREA(0) | PWM_CLK_DIVA(84); // CLKA = MCK / DIVA * 2^PREA = 1 MHz
    PWM->PWM_CH_NUM[1].PWM_CMR  = PWM_CMR_CPRE_CLKA | PWM_CMR_CPOL;  // to have high pulses

    PWM->PWM_CH_NUM[1].PWM_CPRD = 20000;              // 20 ms
    PWM->PWM_CH_NUM[1].PWM_CDTY = 1500;               // inside 0.9 ms - 2.1 ms

    PWM->PWM_ENA = PWM_ENA_CHID1;


    // ---   PWM MOTOR channel 0 ---
    PMC->PMC_PCER0 |= (1u << ID_PIOB);    //  PIOB manipulation
    PMC->PMC_PCER1 |= (1u << (ID_PWM - 32));   //  PWM clock

    PIOB->PIO_PDR   = PIO_PDR_P12;              // disable GPIO control
    PIOB->PIO_ABSR |= PIO_ABSR_P12;             // select Peripheral B (PWMH1)

    //Disable channel during setup
    PWM->PWM_DIS = PWM_DIS_CHID0;

    PWM->PWM_CH_NUM[0].PWM_CMR  = PWM_CMR_CPRE_CLKA | PWM_CMR_CPOL;  // to have high pulses

    PWM->PWM_CH_NUM[0].PWM_CPRD = 20000;              // 20 ms
    PWM->PWM_CH_NUM[0].PWM_CDTY = 10000;               //motor speed

    PWM->PWM_ENA = PWM_ENA_CHID0;


    // --- pc23 for phase init ---
    PMC->PMC_PCER0 |= (1u << ID_PIOC);    
    PIOC->PIO_PER = (1u << 23);    
    PIOC->PIO_OER = (1u << 23);    

    
    uart_init(F_CPU, 115200);

    uint8_t BRP = (F_CPU / (16*BAUD)) - 1; // Baud Rate Prescaler

    uint32_t reg = ((0<<24) | (BRP<<16) | (3<<12) | (1<<8) | (6<<4) | 5);
    /* 
     * - Target bit rate: 250 kbps
     * - Clock frequency: 84 MHz
     * - Time quanta (TQ) per bit: 16
     * 
     * BRP = (F_CPU / (BAUD * TQ)) - 1 = (84,000,000 / (250,000 * 16)) - 1 = 20
     * 
     * Bit timing segments (all decremented by 1 for register):
     * PROPAG = 1 TQ
     * Tprs = 2 * (50+30+DLine) ns = 2 Tcsc 
        => PROPAG = Tprs/Tcsc - 1 = 1

     * Phase1 + Phase2 = 16 TQ - Sync(1 TQ) - PropSeg (2 TQ) = 13 TQ
     * PHASE1 = 7 TQ                -> 6 in register
     * PHASE2 = 6 TQ                -> 5 in register
     * 
     * SJW = 4 TQ                   -> 3 in register
     * SMP = 0 (single sampling)
     * 
     */

    
    can_init_def_tx_rx_mb(reg);

    printf("CAN initialized.\n\r");

    ir_adc_init();

    qdec_tc2_init();

    CAN_MESSAGE rx_msg;


    while (1)
    {   
        //test tresholds code
        // uint16_t sample = ADC->ADC_CDR[IR_ADC_CH] & 0x0FFF;
        // printf("%u\n", sample);


        if (can_receive(&rx_msg, 0) == 0) {
            // printf("RX ID=0x%03X LEN=%d DATA (direction):", rx_msg.id, rx_msg.data_length);
            // const char* val = print_dir(rx_msg.data[0]);
            // printf(" %s", val);
            // printf("\n\r");

            if (rx_msg.id == 0x100) {
                    ball_count = 0;
                    prev_count = 0;
            } else if (rx_msg.id != 0x111) {
                continue;   // ignore other messages
            }

            Direction dir = decode_dir(rx_msg.data[0]);
            switch (dir) {
                case UP:    servo_write(1, 1000); break;  // 0°
                case DOWN:  servo_write(1, 2000); break;  // 180°
                case LEFT:  motor_write(0, 1); break;  // 90° (example)
                case RIGHT: motor_write(0, 0); break;  // 90° (example)
                default:    { 
                                servo_write(1, 1500);
                                motor_write(0, 2); 
                            } break;  // center
            }

        }


        if (ball_count > prev_count) {
            printf("GOL! score: %d\n\r", ball_count);
            prev_count = ball_count;
        }


        int32_t pos = qdec_tc2_get_position();
        // printf("QDEC position: %ld\n\r", pos);

        /* i will use it*/
    }

}

Direction decode_dir(uint8_t val) {
    switch (val) {
        case 0x01: return UP;
        case 0x02: return DOWN;
        case 0x03: return LEFT;
        case 0x04: return RIGHT;
        default:   return NEUTRAL;
    }
}

const char* print_dir(uint8_t val) {
    switch (val) {
        case 0x01: return direction_str[0];
        case 0x02: return direction_str[1];
        case 0x03: return direction_str[2];
        case 0x04: return direction_str[3];
        default:   return direction_str[4];
    }
}

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
    //    – EDGPHA : choose edge phase (A leads B vs B leads A); flip if direction feels inverted
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
