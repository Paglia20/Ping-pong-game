#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "include/UART.h"
#include "include/SRAM.h"
#include "include/decoder.h"

void adc_init (void){

    // clock initialization on OC1A pin (PD5)
    setBit(DDRD, PD5);

    // (COM1A1:0 = 01) : setting OC1A to toggle on compare match 
    clearBit(TCCR1A, COM1A1);
    setBit(TCCR1A,   COM1A0);

    // Put Timer1 in CTC mode with TOP = OCR1A (WGM13:0 = 0100)
    /*
    With WGM12=1 and the rest 0, CTC (Clear Timer on Compare Match) is selected and OCR1A becomes TOP. 
    The timer resets to 0 on each match.
    */
    clearBit(TCCR1B, WGM13);
    setBit( TCCR1B, WGM12);
    clearBit(TCCR1A, WGM11);
    clearBit(TCCR1A, WGM10);


    //Set compare value OCR1A = 0, so the timer matches at count 0 and "toggle on compare"
    //This flag is set in the timer clock cycle after the counter (TCNT1) value matches the Output
    //Compare Register A (OCR1A)
    OCR1AH = 0x00;
    OCR1AL = 0x00;

    //Start Timer1, prescaler = clk/1 (CS12:10 = 001)
    clearBit(TCCR1B, CS12);
    clearBit(TCCR1B, CS11);
    setBit( TCCR1B, CS10);


}