#include "include/board.h"
#include "include/SPI.h"
#include "include/bit_macros.h"
#include <util/delay.h>  

static inline void cs_low(void){  SPI_select(SPI_SLAVE_IO); }
static inline void cs_high(void){ SPI_deselect(SPI_SLAVE_IO); }

// ---- Opcodes ----
#define CMD_BUTTONS   0x04
#define CMD_LED_ONOFF 0x05
#define CMD_LED_PWM   0x06
#define CMD_INFO      0x07

BoardButtons board_init(void){
    BoardButtons b = (BoardButtons){0};
    cs_high();                    
    return b;
}


bool board_read_buttons_raw(uint8_t bytes3_out[3]) {
    if (!bytes3_out) return false;

    cs_low();
    SPI_transmit(CMD_BUTTONS);
    _delay_us(40);                      

    bytes3_out[0] = SPI_receive();      
    _delay_us(2);
    bytes3_out[1] = SPI_receive();      
    _delay_us(2);
    bytes3_out[2] = SPI_receive();      

    cs_high();
    return true;
}

static inline uint8_t bit(const uint8_t x, uint8_t n){ return (x >> n) & 0x1; }

bool board_read_buttons(BoardButtons* out) {
    if (!out) return false;

    uint8_t raw[3];
    if (!board_read_buttons_raw(raw)) return false;

    const uint8_t right = raw[0]; // [0 0 R6 R5 R4 R3 R2 R1]
    const uint8_t left  = raw[1]; // [0 L7 L6 L5 L4 L3 L2 L1]
    const uint8_t nav   = raw[2]; // [0 0 0 Up Down Left Right Btn]

    out->R1 = bit(right,0); out->R2 = bit(right,1); out->R3 = bit(right,2);
    out->R4 = bit(right,3); out->R5 = bit(right,4); out->R6 = bit(right,5);

    out->L1 = bit(left,0);  out->L2 = bit(left,1);  out->L3 = bit(left,2);
    out->L4 = bit(left,3);  out->L5 = bit(left,4);  out->L6 = bit(left,5);
    out->L7 = bit(left,6);

    out->Btn   = bit(nav,0);
    out->Right = bit(nav,1);
    out->Left  = bit(nav,2);
    out->Down  = bit(nav,3);
    out->Up    = bit(nav,4);

    return true;
}


bool board_set_led(uint8_t led, bool on){
    if (led > 5) return false;
    cs_low();
    SPI_transmit(CMD_LED_ONOFF);
    SPI_transmit(led);
    SPI_transmit(on ? 1 : 0);
    cs_high();
    return true;
}


bool board_set_pwm(uint8_t led, uint8_t pwd){
    if (led > 5) return false;
    cs_low();
    SPI_transmit(CMD_LED_PWM);
    SPI_transmit(led);
    SPI_transmit(pwd);
    cs_high();
    return true;
}

bool read_info(void){
    if (!DEBUG) return;

    uint8_t info[35];

    cs_low();
    SPI_transmit(CMD_INFO);
    _delay_us(40);                 

    for (uint8_t i = 0; i < 35; ++i){
        info[i] = SPI_receive();
        if (i != 34) _delay_us(2);
    }
    cs_high();

    
    printf("FW: ");
    for (uint8_t i = 0; i < 19; ++i) putchar(info[i]);
    printf("\r\nSN: ");
    for (uint8_t i = 0; i < 16; ++i) putchar(info[19 + i]);
    printf("\r\n");

    return true;
}