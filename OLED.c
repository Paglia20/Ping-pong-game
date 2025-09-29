
#include "include/OLED.h"
#include "include/SPI.h"

static inline void cs_low(void){ SPI_select(SPI_SLAVE_OLED); }
static inline void cs_high(void){ SPI_deselect(SPI_SLAVE_OLED); }

static inline void dc_cmd(void){ OLED_DC_PORT &= ~(1<<OLED_DC_PIN); }
static inline void dc_data(void){ OLED_DC_PORT |=  (1<<OLED_DC_PIN); }

static inline void set_col_page(uint8_t page, uint8_t col){
    cs_low(); dc_cmd();
    SPI_txrx(0xB0 | (page & 0x07));       // set page
    SPI_txrx(0x00 | (col & 0x0F));        // low nibble col
    SPI_txrx(0x10 | (col >> 4));          // high nibble col
    cs_high();
}

void oled_write_cmd1(uint8_t c){
    cs_low(); 
    dc_cmd(); 
    SPI_txrx(c); 
    cs_high();
}
void oled_write_cmd2(uint8_t c, uint8_t a0){
    cs_low(); 
    dc_cmd(); 
    SPI_txrx(c); 
    SPI_txrx(a0); 
    cs_high();
}
void oled_write_cmd3(uint8_t c, uint8_t a0, uint8_t a1) {
    cs_low(); 
    dc_cmd(); 
    SPI_txrx(c); SPI_txrx(a0); SPI_txrx(a1); 
    cs_high();
}

void oled_write_data(const uint8_t* p, uint16_t n){
    cs_low(); 
    dc_data();
    while (n--) SPI_txrx(*p++);
    cs_high();
}


void OLED_init(void){
    _delay_ms(10);              

    cmd1(0xAE);                 // OFF
    cmd1(0xA1);                 // seg remap
    cmd2(0xDA, 0x12);           // COM pins
    cmd1(0xC8);                 // scan dir
    cmd2(0xA8, 0x3F);           // 1/64
    cmd2(0xD5, 0x80);           // clock
    cmd2(0x81, 0x50);           // contrast
    cmd2(0xD9, 0x21);           // precharge
    cmd1(0x20); cmd1(0x02);     // page addressing
    cmd2(0xDB, 0x30);           // VCOMH
    cmd2(0xAD, 0x00);           // master cfg
    cmd1(0xA4);                 // follow RAM
    cmd1(0xA6);                 // normal
    cmd1(0xAF);                 // ON
}

void OLED_fill(uint8_t pattern){          // pattern=0xFF tutto acceso; 0x00 tutto spento
    for (uint8_t p=0; p<8; ++p){
        set_col_page(p, 0);
        cs_low(); dc_data();
        for (uint8_t x=0; x<128; ++x) SPI_txrx(pattern);
        cs_high();
    }
}