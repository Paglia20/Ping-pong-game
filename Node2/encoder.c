#include "encoder.h"

static int32_t right_limit = 0;
static int32_t left_limit  = 0;
static int32_t MAX_COUNTS  = 0;

// Control state
static volatile int32_t latest_setpoint = 0;   // written from CAN thread, read in ISR

// PI gains & timing
static const float Kp = 3.0f;
static const float Ki = 0.10f;
static const float Ts = 0.020f;                // 20 ms
static float I = 0.0f;                     // integral accumulator


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
        TC_BMR_FILTER    |            // enable filter
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
    time_spinFor(msecs(1400));
    right_limit = qdec_tc2_get_position();


    printf("cal left \n\r");

    motor_write(0, 1, 15000); //left movement
    time_spinFor(msecs(1400));
    left_limit = qdec_tc2_get_position();

    int32_t pos = qdec_tc2_get_position();

    //center
    int32_t middle = (left_limit + right_limit) / 2;

    int32_t cur_pos = qdec_tc2_get_position();
    int32_t error = middle - cur_pos;
    //printf("error: %ld\n\r", error);


    while (abs(error) > DEAD_BAND) {
        cur_pos = qdec_tc2_get_position();
        error = middle - cur_pos;
        //printf("error: %ld\n\r", error);
        if (error < DEAD_BAND) {
            motor_write(0, 0, 8000); //right
            time_spinFor(msecs(NUDGE));
        } else if (error > -DEAD_BAND) {
            motor_write(0, 1, 8000); //left
            time_spinFor(msecs(NUDGE));
        } else {
            break;
        }
        time_spinFor(msecs(5));
    }
    
    motor_write(0, 2, 0); //stop motor

    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;
    left_limit = - (middle - left_limit);
    right_limit = right_limit - middle;

    printf("cal done \n\r");
    printf("left: %ld, right: %ld\n\r", left_limit, right_limit);


    // usable travel with 5% margin
    int32_t max_mag = (abs(left_limit) < abs(right_limit)) ? abs(left_limit) : abs(right_limit);
    MAX_COUNTS = (int32_t)(0.95f * max_mag);
    // start at center
    latest_setpoint = 0;
    I = 0.0f;
}


void set_point(int8_t dir_x) {
    // clamp input
    if (dir_x >  100) dir_x =  100;
    if (dir_x < -100) dir_x = -100;

    int32_t sp = (int32_t)((dir_x / 100.0f) * MAX_COUNTS);
    latest_setpoint = sp;   // the 20ms control task will read this

    // printf("Setpoint updated to %ld for dir_x %d\n\r", sp, dir_x);
}


void update_motor(void) {
    const int DUTY_MAX = 20000;

    int32_t pos_actual = qdec_tc2_get_position();
    int32_t err = latest_setpoint - pos_actual;

    // Dentro banda morta: stop e scarica un filo l’integrale per evitare drift
    if (abs(err) <= DEAD_BAND) {
        I *= 0.98f;                     // leak lento
        if (fabsf(I) < 1.0f) I = 0.0f;  // snap to zero
        motor_write(0, 2, 0);
        return;
    }

    // Proporzionale (solo err intero → float una volta sola)
    float P = Kp * (float)err;

    // Controllo provvisorio senza anti-windup
    float u_noI = P;
    float u_withI = u_noI + Ki * I;

    // Saturazione “duty richiesta”
    float u_cmd = u_withI;
    if (u_cmd >  (float)DUTY_MAX) u_cmd = (float)DUTY_MAX;
    if (u_cmd < -(float)DUTY_MAX) u_cmd = -(float)DUTY_MAX;

    // === Conditional integration ===
    // Calcola quale sarebbe il duty senza clamp per capire se sei saturo.
    bool saturated_pos = (u_withI >  (float)DUTY_MAX);
    bool saturated_neg = (u_withI < -(float)DUTY_MAX);

    // Err con segno del comando: se saturo e l’integrale spingerebbe nella stessa direzione, NON integrare.
    bool integrate;
    if (saturated_pos)      integrate = ((float)err < 0); // integra solo se riduce u (errore < 0)
    else if (saturated_neg) integrate = ((float)err > 0); // integra solo se riduce u (errore > 0)
    else                    integrate = true;

    if (integrate) {
        I += Ts * (float)err;
        // clamp integrale, più stretto del duty max per avere spazio al P
        const float I_MAX = 15000.0f;
        if (I >  I_MAX) I =  I_MAX;
        if (I < -I_MAX) I = -I_MAX;
    } else {
        // piccola perdita quando saturo per evitare sticking
        I *= 0.999f;
    }

    // Ricomputa il comando finale dopo l’eventuale aggiornamento di I
    float u = Kp * (float)err + Ki * I;

    int dir, duty;
    if (u >= 0.0f) { dir = 0; duty = (int)u; }   // 0 = destra
    else           { dir = 1; duty = (int)(-u); }// 1 = sinistra

    if (duty > DUTY_MAX) duty = DUTY_MAX;
    if (duty < 0)        duty = 0;

    motor_write(0, dir, duty);
}


void control_timer_init(void)
{
    // 1) Enable peripheral clock for TC0
    PMC->PMC_PCER0 |= (1 << ID_TC0);

    // 2) Configure TC0 Channel 0 for compare match at 50 Hz
    TcChannel *ch = &TC0->TC_CHANNEL[0];

    ch->TC_CMR = 
        TC_CMR_TCCLKS_TIMER_CLOCK4 |   // MCK/128 → 84 MHz / 128 = 656 250 Hz
        TC_CMR_WAVE |                  // waveform mode
        TC_CMR_WAVSEL_UP_RC;           // reset on RC compare

    // 3) RC = ticks per period = 656 250 Hz * 0.02 s = 13 125
    ch->TC_RC = 13125;

    // 4) Enable interrupt on RC compare
    ch->TC_IER = TC_IER_CPCS;          // interrupt on RC match
    ch->TC_IDR = ~TC_IER_CPCS;

    // 5) Enable NVIC interrupt
    NVIC_EnableIRQ(TC0_IRQn);

    // 6) Start timer
    ch->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
}

void TC0_Handler(void)
{
    // mandatory: clear interrupt flag
    TC0->TC_CHANNEL[0].TC_SR;
    update_motor();
}


