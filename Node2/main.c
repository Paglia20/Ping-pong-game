/*
 * Remember to update the Makefile with the (relative) path to the uart.c file.
 * This starter code will not compile until the UART file has been included in the Makefile. 
 * If you get somewhat cryptic errors referencing functions such as _sbrk, 
 * _close_r, _write_r, _fstat etc, you have most likely not done that correctly.

 * If you get errors such as "arm-none-eabi-gcc: no such file", you may need to reinstall the arm gcc packages using
 * apt or your favorite package manager.
 */

#include "main.h"

volatile uint32_t ball_count = 0;
volatile uint32_t prev_count = 0;

const char* direction_str[] = {
    "UP", "DOWN", "LEFT", "RIGHT", "NEUTRAL"
};

static inline void servo_write(uint32_t ch, uint16_t us)
{
    if (us < 900)  us = 900;     
    if (us > 2100) us = 2100;   
    PWM->PWM_CH_NUM[ch].PWM_CDTYUPD = us;

    printf("servo: %d\n\r", PWM->PWM_CH_NUM[ch].PWM_CDTY);

}

const char* print_dir(uint8_t val);

Direction decode_dir(uint8_t val);


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



    //PWM
    PMC->PMC_PCER0 |= (1u << ID_PIOB);    //  PIOB manipulation
    PMC->PMC_PCER1 |= (1 << (ID_PWM - 32));   //  PWM clock

    // choosing PB13
    PIOB->PIO_PDR   = PIO_PDR_P13;              // disable GPIO control
    PIOB->PIO_ABSR |= PIO_ABSR_P13;             // select Peripheral B (PWMH1)

    //Disable channel during setup
    PWM->PWM_DIS = PWM_DIS_CHID1;

    // Set Channel 1 to use CLKA
    PWM->PWM_CLK = PWM_CLK_PREA(0) | PWM_CLK_DIVA(84); // CLKA = MCK / DIVA * 2^PREA = 1 MHz
    PWM->PWM_CH_NUM[1].PWM_CMR  = PWM_CMR_CPRE_CLKA;  

    PWM->PWM_CH_NUM[1].PWM_CPRD = 20000;              // 20 ms
    PWM->PWM_CH_NUM[1].PWM_CDTY = 1500;               // inside 0.9 ms - 2.1 ms

    PWM->PWM_ENA = PWM_ENA_CHID1;

    // Verify by measuring pulse width using the capture and cursor functions.


    //CDTYUPD register is used to update the duty cycle on the fly
    // 0°   → PWM->PWM_CH_NUM[0].PWM_CDTYUPD = 1000;   // 1.0 ms - 5% (duty cycle)
    // 90°  → PWM->PWM_CH_NUM[0].PWM_CDTYUPD = 1500;   // 1.5 ms - 7,5%
    // 180° → PWM->PWM_CH_NUM[0].PWM_CDTYUPD = 2000;   // 2.0 ms - 10%
    
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

    CAN_MESSAGE rx_msg;

    while (1)
    {   
        //test tresholds code
        uint16_t sample = ADC->ADC_CDR[IR_ADC_CH] & 0x0FFF;
        printf("%u\n", sample);
        delay_ms(200);


        if (can_receive(&rx_msg, 0) == 0) {
            printf("RX ID=0x%03X LEN=%d DATA (direction):", rx_msg.id, rx_msg.data_length);
            const char* val = print_dir(rx_msg.data[0]);
            printf(" %s", val);
            printf("\n\r");


            Direction dir = decode_dir(rx_msg.data[0]);
            switch (dir) {
                case UP:    servo_write(1, 1000); break;  // 0°
                case DOWN:  servo_write(1, 2000); break;  // 180°
                case LEFT:  servo_write(1, 1500); break;  // 90° (example)
                case RIGHT: servo_write(1, 1500); break;  // 90° (example)
                default:    servo_write(1, 1500); break;  // center
            }

        }


        if (ball_count > prev_count) {
            printf("GOL! score: %d\n\r", ball_count);
            prev_count = ball_count;
        }
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
