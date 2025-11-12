/*
 * Remember to update the Makefile with the (relative) path to the uart.c file.
 * This starter code will not compile until the UART file has been included in the Makefile. 
 * If you get somewhat cryptic errors referencing functions such as _sbrk, 
 * _close_r, _write_r, _fstat etc, you have most likely not done that correctly.

 * If you get errors such as "arm-none-eabi-gcc: no such file", you may need to reinstall the arm gcc packages using
 * apt or your favorite package manager.
 */

#include "main.h"

volatile uint32_t ball_count;
volatile uint32_t prev_count;
volatile uint8_t  ir_enable;    
volatile uint8_t solenoid_enable;


const char* direction_str[] = {
    "UP", "DOWN", "LEFT", "RIGHT", "NEUTRAL"
};

static inline void pb25_pulse(void){
    PIOB -> PIO_CODR  = PIO_PB25; //set high
    for (volatile uint32_t i = 0; i < 400000; i++); //delay
    PIOB -> PIO_SODR  = PIO_PB25; //set low
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


    init_pmw_servo();

    init_pmw_motor();


    // --- pc23 for phase init ---
    PMC->PMC_PCER0 |= (1u << ID_PIOC);    
    PIOC->PIO_PER = (1u << 23);    
    PIOC->PIO_OER = (1u << 23);    

    // PB25
    PMC -> PMC_PCER0 |= (1 << ID_PIOB);
    PIOB -> PIO_PER   = PIO_PB25;    // Enable
    PIOB -> PIO_OER   = PIO_PB25;    // Output
    PIOB -> PIO_SODR  = PIO_PB25;


    
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

    
    //can_init_def_tx_rx_mb(reg);

    CanInit init = {
        .reg = reg
    };
    
    can_init(init, 0);

    printf("CAN initialized.\n\r");

    ir_adc_init();

    CanMsg rx_msg;

    ir_enable = 1;
    solenoid_enable = 1;

    encode_init();
    control_timer_init(); 
    printf("Encoder calibrated, control timer initialized.\n\r");

    while (1)
    {   
        //test tresholds code
        // uint16_t sample = ADC->ADC_CDR[IR_ADC_CH] & 0x0FFF;
        // printf("%u\n\r", sample);

        if (!ir_enable) {
            servo_write(1, 1500);
            motor_write(0, 2, 0); 
        }

        if ((ball_count > prev_count) && (ir_enable == 1)) {
            printf("GOL! score: %d\n\r", ball_count);
            prev_count = ball_count;

            //stop motors
            ir_enable = 0;            

            CanMsg tx = {
                .id  = 0x03,
                .length = 1,
                .byte = {0x01}
            };

            printf("sending finish");

            can_tx(tx);

        } 
        else if (can_rx(&rx_msg) == 1) {
            printf("RX ID=0x%03X LEN=%d DATA (direction):", rx_msg.id, rx_msg.length);
            const char* val = print_dir(rx_msg.byte[0]);
            printf(" %s , X: %d, Y: %d, button : %d", val, (int8_t) rx_msg.byte[1], (int8_t) rx_msg.byte[2], rx_msg.byte[3]);
            printf("\n\r");

            if (rx_msg.id == 0x100) {
                    ball_count = 0;
                    prev_count = 0;
                    ir_enable = 1;
            } else if (rx_msg.id != 0x111) {
                continue;   // ignore other messages
            }

            if (rx_msg.byte[3] == 1 && solenoid_enable == 1) {
                pb25_pulse();
                solenoid_enable = 0;
            }

            if (rx_msg.byte[3] == 0 && solenoid_enable == 0) {
                solenoid_enable = 1;
            }

            Direction dir = decode_dir(rx_msg.byte[0]);
            switch (dir) {
                case UP:    servo_write(1, 1000); break;  // 0°
                case DOWN:  servo_write(1, 2000); break;  // 180°
                //case NEUTRAL: servo_write(1, 1500); break;  // 90°
                default:    {
                    servo_write(1, 1500);
                    set_point((int8_t) rx_msg.byte[1]);
                
                } break; 
            }

        }

        
    }

}

void init_pmw_servo(void){
    // --- PWM SERVO channel 1 del PWM ---choose edge phase (
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
}

void init_pmw_motor(void){
    // ---   PWM MOTOR channel 0 ---
    PMC->PMC_PCER0 |= (1u << ID_PIOB);    //  PIOB manipulation
    PMC->PMC_PCER1 |= (1u << (ID_PWM - 32));   //  PWM clock

    PIOB->PIO_PDR   = PIO_PDR_P12;              // disable GPIO control
    PIOB->PIO_ABSR |= PIO_ABSR_P12;             // select Peripheral B (PWMH0)

    //Disable channel during setup
    PWM->PWM_DIS = PWM_DIS_CHID0;

    PWM->PWM_CH_NUM[0].PWM_CMR  = PWM_CMR_CPRE_CLKA | PWM_CMR_CPOL;  // to have high pulses

    PWM->PWM_CH_NUM[0].PWM_CPRD = 20000;              // 20 ms
    PWM->PWM_CH_NUM[0].PWM_CDTY = 0;               //motor speed

    PWM->PWM_ENA = PWM_ENA_CHID0;
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


