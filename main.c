/*
 * GccApplication1.c
 * Created: 02.09.2025
 * Author: Paglia20
 */

#define F_CPU 4915200UL  // 4.9152 MHz
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "UART.h"

// Hook stdio to UART
static int uart_putchar(char c, FILE *stream) { uart_putc(c); return 0; }
static int uart_getchar(FILE *stream) { return uart_getc(); }

//FDEV_SETUP_STREAM sets up a FILE stream to use custom input/output functions (e.g., UART) for stdin and stdout.
// its like fdevopen but for AVR
static FILE uart_stdio = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

static void exercise1(void) {
    // read one byte if available, send back the next ASCII (A->B, '0'->'1', etc.)
    if (uart_available()) {
        char c = uart_getc();
        uart_putc(c + 1);
    }
}

int main(void) {
    UART_init(F_CPU, 9600);    // 9600 8N1
    stdout = &uart_stdio;
    stdin  = &uart_stdio;

    printf("printf is working?\r\n");

    for (;;) {
        exercise1();
        _delay_ms(10);
    }
}