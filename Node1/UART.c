#include <avr/io.h>
#include <stddef.h>  
#include <stdio.h>
#include <avr/pgmspace.h>



#include "../include/UART.h"
#include "../include/bit_macros.h"

// Hook stdio to UART
static int uart_putchar(char c, FILE *stream) {
    if (c == '\n') uart_putc('\r');   // optional: add CR before LF
    uart_putc(c);
    return 0;
}

static int uart_getchar(FILE *stream) {
    return uart_getc();  // blocking read
}

void UART_init(uint32_t clock_hz, uint32_t baud) {
    
    uint16_t ubrr = (uint16_t)(clock_hz / (16UL * baud) - 1UL);

    //UBRR0 sets the baud rate for UART communication. The value written to UBRR0H and UBRR0L determines the speed of data transmission
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr);

    UCSR0A = 0;                                // normal speed
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
    //UCSR0C = (1 << URSEL0) | (1 << UCSZ01) | (1 << UCSZ00);


    //if we want 2 stop bits:
    UCSR0C = (1<<URSEL0)|(1<<USBS0)|(3<<UCSZ00);

    //UCSZ01 = channel-specific bit for USART0

    set_bit(UCSR0B,RXEN0);		//RXEN enables the receiver.
	set_bit(UCSR0B,TXEN0);		//TXEN enables the transmitter.

    fdevopen(uart_putchar, uart_getchar); // Hook up putc and getc to stdio

}

void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0))) { }  //look for UDRE0 (bit inside UCSR0A) to know if buffer is empty
    UDR0 = c;
    //printf("Sent char: %c\n", c);
}

void uart_puts(const char *s) {
    if (s == NULL) return;  // Guard for NULL pointer
    while (*s) uart_putc(*s++);
}

int uart_available(void) {
    return (UCSR0A & (1 << RXC0)) != 0;
}

char uart_getc(void) {
    while (!uart_available()) {; /* Wait for data to be available*/}
    char out = UDR0;
    //printf("Received char: %c\n", out); 

    return out; // Return the received character
}



void uart_puts_P(const char *progmem_s) {
    char c;
    while ((c = pgm_read_byte(progmem_s++))) {
        uart_putc(c);
    }
}

