#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <stdint.h>

// ---- Slave Select pins ----
#define CS_OLED_PORT  PORTB
#define CS_OLED_DDR   DDRB
#define CS_OLED_PIN   PB0

#define CS_IO_PORT    PORTB
#define CS_IO_DDR     DDRB
#define CS_IO_PIN     PB1

#define DC_PORT PORTD
#define DC_DDR  DDRD
#define DC_PIN  PD2   


typedef enum {
    SPI_SLAVE_OLED = 0,
    SPI_SLAVE_IO   = 1
} spi_slave_t;

void    SPI_init(void);                       
void    SPI_select(spi_slave_t slave);        
void    SPI_deselect(spi_slave_t slave);      

void    SPI_transmit(uint8_t data);           
uint8_t SPI_receive(void);                    
uint8_t SPI_txrx(uint8_t data);               

void    SPI_write_n(const uint8_t* data, uint16_t n);
void    SPI_read_n(uint8_t* buffer, uint16_t n);

#endif // SPI_H