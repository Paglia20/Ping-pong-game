#include "include/MCP2515.h"
#include "include/SPI.h"    // usa la tua SPI_txrx() e SPI_select/deselect()
#include "include/bit_macros.h"

// PORTB &= ~(1 << CONTR_CS_PIN) // SELECT
static inline void cs_low(void)  {SPI_deselect(SPI_SLAVE_CONTR); SPI_select(SPI_SLAVE_CONTR); }
// PORTB |=  (1 << CONTR_CS_PIN) // DESELECT
static inline void cs_high(void) { SPI_deselect(SPI_SLAVE_CONTR); }


void MCP_init(void) {
    // 1) GPIO
    CONTR_CS_DDR  |=  (1 << CONTR_CS_PIN);   // CS output
    cs_high();                               
    CONTR_INT_DDR &= ~(1 << CONTR_INT_PIN);  // INT input
    CONTR_INT_PORT |= (1 << CONTR_INT_PIN);  // pull-up on INT

    // 2) SPI
    SPI_init();

    uint8_t value;
}


void MCP_reset(void) {
    cs_low();
    SPI_txrx(MCP_RESET);                  // 0xC0 - RESET (datasheet §12.1)
    cs_high();
    _delay_ms(10);                        

    uint8_t value;


    // Self - test
    value = MCP_read ( MCP_CANSTAT);
    if (( value & MODE_MASK ) != MODE_CONFIG ) {
    printf ("MCP NOT in configuration after reset !\n %d ", value ) ;
    } else if (DEBUG)
    {
        printf("mode %d\n", value);
    }    
                   
}

//read a register from MCP2515
uint8_t MCP_read(uint8_t addr) {
    cs_low();
    SPI_txrx(MCP_READ);                   
    SPI_txrx(addr);
    uint8_t data = SPI_txrx(0x00);
    cs_high();
    return data;
}

void MCP_write(uint8_t addr, uint8_t data) {
    cs_low();
    SPI_txrx(MCP_WRITE);                  
    SPI_txrx(addr);
    SPI_txrx(data);
    cs_high();
}

void MCP_bit_modify(uint8_t addr, uint8_t mask, uint8_t data) {
    cs_low();
    SPI_txrx(MCP_BITMOD);                 // 0x05 - BIT MODIFY (§12.5)
    SPI_txrx(addr);
    SPI_txrx(mask);
    SPI_txrx(data);
    cs_high();
}

void MCP_rts(uint8_t buffer_index) {
    if (buffer_index > 2) return;       //TX0, TX1, TX2
    cs_low();
    SPI_txrx(MCP_RTS_BASE | (1 << buffer_index)); 
    cs_high();
}

uint8_t MCP_read_status(void) {
    cs_low();
    SPI_txrx(MCP_READ_STATUS);
    uint8_t status = SPI_txrx(0x00);
    cs_high();
    return status;
}


void MCP_set_mode(uint8_t mode) {
    MCP_bit_modify(MCP_CANCTRL, MODE_MASK, mode);
    
    // while ((MCP_read(MCP_CANSTAT) & MODE_MASK) != mode){
    //     uint8_t current_mode = MCP_read(MCP_CANSTAT) & MODE_MASK;
    //     printf("Current mode: 0x%02X, Desired mode: 0x%02X\n", current_mode, mode);

    // }; // wait until mode is set
    

    //look into this
}

void MCP_enable_interrupts(uint8_t mask) {
    MCP_write(MCP_CANINTE, mask);
}

uint8_t MCP_get_interrupt_flags(void) {
    return MCP_read(MCP_CANINTF);
}

void MCP_clear_interrupt_flags(uint8_t mask) {
    MCP_bit_modify(MCP_CANINTF, mask, 0x00); 
}