#pragma once
#include <stdint.h>

void UART_init(uint32_t clock_hz, uint32_t baud);
void uart_putc(char c);
void uart_puts(const char *s);
int  uart_available(void);
char uart_getc(void);  // blocking
void uart_puts_P(const char *progmem_s);