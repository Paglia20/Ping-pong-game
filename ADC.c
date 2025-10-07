#include "include/UART.h"
#include "include/SRAM.h"
#include "include/decoder.h"
#include "include/bit_macros.h"
#include "include/ADC.h"

#define adc_address 0x1000
#define num_channels 3

//ADC storage
static volatile uint8_t * const adcVal = (uint8_t *)adc_address;

//ADC values out
static uint8_t adcData[num_channels];


void adc_init (void){

    // clock initialization on OC1A pin (PD5)
    set_bit(DDRD, PD5);

    // (COM1A1:0 = 01) : setting OC1A to toggle on compare match 
    clear_bit(TCCR1A, COM1A1);
    set_bit(TCCR1A,   COM1A0);

    // Put Timer1 in CTC mode with TOP = OCR1A (WGM13:0 = 0100)
    /*
    With WGM12=1 and the rest 0, CTC (Clear Timer on Compare Match) is selected and OCR1A becomes TOP. 
    The timer resets to 0 on each match.
    */
    clear_bit(TCCR1B, WGM13);
    set_bit( TCCR1B, WGM12);
    clear_bit(TCCR1A, WGM11);
    clear_bit(TCCR1A, WGM10);


    //Set compare value OCR1A = 0, so the timer matches at count 0 and "toggle on compare"
    //This flag is set in the timer clock cycle after the counter (TCNT1) value matches the Output
    //Compare Register A (OCR1A)
    OCR1AH = 0x00;
    OCR1AL = 0x00;

    //(CS12:10 = 001) : // No prescaling 
    clear_bit(TCCR1B, CS12);
    clear_bit(TCCR1B, CS11);
    set_bit( TCCR1B, CS10);


    // PD4 = BUSY input
    clear_bit(DDRD, PD4);

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

    if (!wait_busy(0,10))  { /*printf("ADC busy timeout waiting for LOW\n");*/ } 
    if (!wait_busy(1,50))  { /*printf("ADC busy timeout waiting for HIGH\n");*/ }

    //no busy
    //_delay_us(8);  // ~7.3 µs @ 2.4576 MHz

	
	for (uint8_t i = 0; i < num_channels; i++) {
		adcData[i] = *adcVal;
        //printf(" val %d: %d\r\n", i, adcData[i]);
	}



	return adcData;
}	



