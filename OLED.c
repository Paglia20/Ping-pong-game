
#include "include/OLED.h"
#include "include/SPI.h"

static inline void cs_low(void){ SPI_select(SPI_SLAVE_OLED); }
static inline void cs_high(void){ SPI_deselect(SPI_SLAVE_OLED); }

static inline void dc_cmd(void){ DC_PORT &= ~(1<<DC_PIN); }
static inline void dc_data(void){ DC_PORT |=  (1<<DC_PIN); }

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

    oled_write_cmd1(0xAE);                 // OFF
    oled_write_cmd1(0xA1);                 // seg remap
    oled_write_cmd2(0xDA, 0x12);           // COM pins
    oled_write_cmd1(0xC8);                 // scan dir
    oled_write_cmd2(0xA8, 0x3F);           // 1/64
    oled_write_cmd2(0xD5, 0x80);           // clock
    oled_write_cmd2(0x81, 0x50);           // contrast
    oled_write_cmd2(0xD9, 0x21);           // precharge

    oled_write_cmd2(0x20, 0x02); // page addressing

    oled_write_cmd2(0xDB, 0x30);           // VCOMH
    oled_write_cmd2(0xAD, 0x00);           // master cfg
    oled_write_cmd1(0xA4);                 // follow RAM
    oled_write_cmd1(0xA6);                 // normal
    oled_write_cmd1(0xAF);                 // ON
    //_delay_ms(10);

    oled_write_cmd1(0xA5);   // Entire Display ON
    _delay_ms(200);
    oled_write_cmd1(0xA4);   // back to RAM
}

void OLED_fill(uint8_t pattern){          // pattern=0xFF tutto acceso; 0x00 tutto spento
    for (uint8_t p=0; p<8; ++p){
        set_col_page(p, 0);
        cs_low(); dc_data();
        for (uint8_t x=0; x<128; ++x) SPI_txrx(pattern);
        cs_high();
    }
}


//remember to check
//	•	IREF = resistenza a massa.
//  •	VCOMH = condensatore a massa.