/*
 * GccApplication1.c
 *
 * Created: 02.09.2025 14:20:17
 * Author : muhammao
 */ 

#define F_CPU 4915200UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "bit_macros.h"
#include "UART_driver.h"


void exercise1(void) {

	//UART

	put_char(get_char() + 1);		//Returns the character next in the alphabet.

	printf("\nhello world!\n");

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
