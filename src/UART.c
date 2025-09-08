#include <avr/io.h>
#include <stddef.h>  

#include "../include/UART.h">


void UART_init(uint32_t clock_hz, uint32_t baud) {
    uint16_t ubrr = (uint16_t)(clock_hz / (16UL * baud) - 1UL);

    //UBRR0 sets the baud rate for UART communication. The value written to UBRR0H and UBRR0L determines the speed of data transmission
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr);

    UCSR0A = 0;                                // normal speed
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);      // enable RX, TX
    /* 
    ABOUT AVR SEMANTIC:
    RXEN0 = bit position for RX Enable
    (1 << RXEN0) = move binary 1 left by RXEN0 bits → register becomes 00001000 (assuming RXEN0 is 3)
    (1 << RXEN0) | (1 << TXEN0) = bitwise OR operation to combine RX and TX enable bits
    
    UCSR0 - status and control register for USART0
    A) contains state info
    B) contains enables
    C) contains config info
    */
 
    // Frame: 8 data, 1 stop, no parity
    // IMPORTANT on ATmega162: set URSEL0 when writing UCSR0C (shared with UBRR0H) (p 184)
    UCSR0C = (1 << URSEL0) | (1 << UCSZ01) | (1 << UCSZ00);


    //if we want 2 stop bits:
    //UCSR0C = (1 << URSEL0) | (1 << UCSZ01)|  (1 << UCSZ00) | (1 << USBS0);


    //UCSZ01 = channel-specific bit for USART0
}

void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0))) { }  //look for UDRE0 (bit inside UCSR0A) to know if buffer is empty
    UDR0 = c;
}

void uart_puts(const char *s) {
    if (s == NULL) return;  // Guard for NULL pointer
    while (*s) uart_putc(*s++);
}

int uart_available(void) {
    return (UCSR0A & (1 << RXC0)) != 0;
}

char uart_getc(void) {
    while (!uart_available()) { }
    return UDR0; // Return the received character
}