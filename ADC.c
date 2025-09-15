#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "include/UART.h"
#include "include/SRAM.h"
#include "include/decoder.h"


#define adc_address 0x1000
#define num_channels 4

//ADC storage
static volatile uint8_t * const adcVal = (uint8_t *)adc_address;

// TODO collega il pin BUSY del MAX156 a PD4
#define ADC_BUSY_PINR  PIND
#define ADC_BUSY_BIT   PD4      

//ADC values out
static uint8_t adcData[num_channels];


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

    //(CS12:10 = 001) : // prescaler 1
    clearBit(TCCR1B, CS12);
    clearBit(TCCR1B, CS11);
    setBit( TCCR1B, CS10);


    // PD4 = BUSY input
    clearBit(DDRD, PD4);

}

static inline uint8_t wait_busy(uint8_t want_high, uint16_t timeout_us) {
    while (timeout_us--) {
        uint8_t is_high = (ADC_BUSY_PINR & (1 << ADC_BUSY_BIT)) != 0;
        if (is_high == want_high) return 1;
        _delay_us(1);
    }
    return 0;
}

uint8_t * adc_read(){	
    //START CONVERSION
    *adcVal = 0;

    if (!wait_busy(0,10))  { printf("ADC busy timeout waiting for LOW\n"); } 
    if (!wait_busy(1,50))  { printf("ADC busy timeout waiting for HIGH\n"); }

    //no busy
    // _delay_us(8);  // ~7.3 µs @ 2.4576 MHz

	
	for (uint8_t i = 0; i < num_channels; i++) {
		adcData[i] = *adcVal;
	}
	return adcData;
}	



//note per domani
// decidere MODE: Single ended unipolar? 
// testare CS
// testare/collegare clock PD5
// collegare BUSY a PD4
//EXTERNAL VOLTAGE???

//testare adc

//quando leggi un array, quindi un pointer, ti basta poi fare *p o *(p+1)