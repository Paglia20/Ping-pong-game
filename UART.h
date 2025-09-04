/*
 * UART.h
 *
 * Created: 03.09.2016 16:32:57
 *  Author: Paglia20
 */ 


#ifndef UART_H_
#define UART_H_

void UART_init(unsigned long clock_speed);
int put_char(unsigned char c);
int get_char(void);

#endif