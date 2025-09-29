
#include "include/OLED.h"
#include "include/SPI.h"
#include "include/fonts.h"

#define FONT5_WIDTH 5
#define FONT5_SPACING 1   // one blank column between chars

static uint8_t cursor_page = 0;
static uint8_t cursor_col  = 0;

static inline void cs_low(void){ SPI_select(SPI_SLAVE_OLED); }
static inline void cs_high(void){ SPI_deselect(SPI_SLAVE_OLED); }

static inline void dc_cmd(void){ DC_PORT &= ~(1<<DC_PIN); }
static inline void dc_data(void){ DC_PORT |=  (1<<DC_PIN); }


// The (page & 0x07) masks to 3 bits, so even if you pass >7, only the lowest
// 3 bits are used → avoids sending an invalid command.
static inline void set_page(uint8_t page){
    cs_low(); dc_cmd();
    SPI_txrx(0xB0 | (page & 0x07));       // set page
    cs_high();
}

//Again the masks to not send bits outside the 4-bit fields.
static inline void set_col(uint8_t col){
    cs_low(); dc_cmd();
    SPI_txrx(0x00 | (col & 0x0F));             // 00 low nibble = bits [3:0]
    SPI_txrx(0x10 | ((col >> 4) & 0x0F));      // 10 high nibble = bits [6:4]
    cs_high();
}

static inline void set_col_page(uint8_t page, uint8_t col){
    set_page(page);
    set_col(col);
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
    //DC already set as output in SPI_init()

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
}

void OLED_fill_strips (void){          // pattern=0xFF tutto acceso; 0x00 tutto spento
    for (uint8_t p=0; p<8; ++p){
        set_col_page(p, 0);
        cs_low(); dc_data();
        for (uint8_t x=0; x<128; ++x) {
            if (x & 1){
                SPI_txrx(0xFF);
            }else{
                SPI_txrx(0x00);
            }          
        }
        cs_high();
    }
}

void oled_home(void){
    cursor_page = 0;
    cursor_col  = 0;
    set_col_page(0,0);   // posizione HW del controller
}

void oled_set_cursor(uint8_t page, uint8_t col){
    if (page > 7) page = 7;
    if (col  > 127) col = 127;
    cursor_page = page;
    cursor_col  = col;

    //perchè set cursor non richiama set_col_page?
    set_col_page(cursor_page, cursor_col);   //nel dubbio lo metto
}

// Advance to next line (8 px down), col=0
void oled_newline(void){
    cursor_col = 0;
    cursor_page = (cursor_page < 7) ? (cursor_page + 1) : 0;
}


void oled_putchar(char c) {
    // Handle control chars
    if (c == '\n') { oled_newline(); return; }
    if (c == '\r') { cursor_col = 0; return; }

    // Only printable ASCII 0x20..0x7E are in font5
    if ((uint8_t)c < 0x20 || (uint8_t)c > 0x7E) {
        c = ' '; // replace unsupported with space
    }

    // Wrap to next line if not enough space for the glyph + spacing
    uint8_t needed = FONT5_WIDTH + FONT5_SPACING;
    if (cursor_col > (uint8_t)(127 - needed + 1)) { // +1 because col is inclusive
        oled_newline();
    }

    // Position the "cursor"
    set_col_page(cursor_page, cursor_col);

    // Burst-write the 5 columns of the glyph from PROGMEM
    cs_low(); 
    dc_data();
    uint8_t idx = (uint8_t)c - 0x20;            // 0..94
    for (uint8_t i = 0; i < FONT5_WIDTH; i++) {
        uint8_t colbyte = pgm_read_byte(&font5[idx][i]);
        SPI_txrx(colbyte);
    }
    // one blank column as spacing
    SPI_txrx(0x00);
    cs_high();

    // Advance cursor
    cursor_col += needed;
}


void oled_print(const char *s) {
    while (*s) {
        oled_putchar(*s++);
    }
}


void oled_clear_line(uint8_t page){
    if (page > 7) return;         
    set_col_page(page, 0);         // vai a col=0 di quella pagina
    cs_low(); dc_data();
    for (uint8_t x = 0; x < 128; x++) {
        SPI_txrx(0x00);            // scrivi byte vuoto = 8 pixel spenti
    }
    cs_high();

    // reset cursore logico a inizio linea
    cursor_page = page;
    cursor_col  = 0;
}


void oled_clear (void) {
    for (size_t i = 0; i < 8; i++){
        oled_clear_line(i);
    }
}

