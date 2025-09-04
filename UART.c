#include <avr/io.h>
#include "UART.h"

void UART_init(uint32_t clock_hz, uint32_t baud) {
    uint16_t ubrr = (uint16_t)(clock_hz / (16UL * baud) - 1UL);

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr);

    UCSR0A = 0;                                // normal speed
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);      // enable RX, TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);    // 8 data, 1 stop, no parity (8N1)
}

void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0))) { }
    UDR0 = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

int uart_available(void) {
    return (UCSR0A & (1 << RXC0)) != 0;
}

char uart_getc(void) {
    while (!uart_available()) { }
    return UDR0;
}