/*
 * GccApplication1.c
 *
 * Created: 02.09.2025 14:20:17
 *  Author: Paglia20
 */ 

#define F_CPU 4915200UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "bit_macros.h"
#include "UART.h"

#define FOSC 1843200

void exercise1(void) {
	UART_init(FOSC);
	//UART
	put_char(get_char() + 1);		

	printf("h");

}

int main(void)
{
   DDRA |= (1 << DDA5);
   while (1)
   {
	   exercise1();
	   _delay_ms(1000);
   }
}
