#include "../include/SPI.h"

void SPI_init(void) {
    // MOSI (PB5), SCK (PB7), SS_hw (PB4) -> output
    DDRB  |= (1<<DDB5) | (1<<DDB7) | (1<<DDB4);
    // MISO (PB6) input
    DDRB  &= ~(1<<DDB6); 

    // D/C output
    DC_DDR |= (1<<DC_PIN);

 

    // /CS come output e a riposo alto //port |= (1<<pin) 
    CS_OLED_DDR |= (1<<CS_OLED_PIN);
    CS_IO_DDR   |= (1<<CS_IO_PIN);
    CS_OLED_PORT |= (1<<CS_OLED_PIN);
    CS_IO_PORT   |= (1<<CS_IO_PIN);

   
    // disabilita slave hardware
    PORTB |= (1<<PB4);

    // Enable SPI, Master, MODE0 already on (CPOL=0, CPHA=0), only SPR0 means SCK=fosc/16
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1);
}

void SPI_select(spi_slave_t slave) {
    if (slave == SPI_SLAVE_OLED) {
        // /CS OLED LOW = selezionato
        CS_OLED_PORT &= ~(1 << CS_OLED_PIN);
    } else if (slave == SPI_SLAVE_CONTR)
    {
        CONTR_CS_PORT &= ~(1 << CONTR_CS_PIN);
    }
    else {
        // /CS IO LOW = selezionato
        CS_IO_PORT   &= ~(1 << CS_IO_PIN);
    }
}

void SPI_deselect(spi_slave_t slave) {
    CS_IO_PORT   |=  (1 << CS_IO_PIN);

    CS_OLED_PORT |=  (1 << CS_OLED_PIN);

    CONTR_CS_PORT |=  (1 << CONTR_CS_PIN);
}

uint8_t SPI_txrx(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1<<SPIF))) {;}
    return SPDR;
}

void SPI_transmit(uint8_t data) {
    (void)SPI_txrx(data); // scarta il byte ricevuto
}

uint8_t SPI_receive(void){
    return SPI_txrx(0xFF); // dummy per generare il clock
}

//make sure to call SPI_select and SPI_deselect around these functions
void SPI_write_n(const uint8_t* data, uint16_t n) {
    for (uint16_t i = 0; i < n; i++) {
        (void) SPI_txrx(*data++);
    }
}

void SPI_read_n(uint8_t* buffer, uint16_t n) {
    for (uint16_t i = 0; i < n; i++) {
        *buffer++ = SPI_txrx(0xFF); 
    }
}

