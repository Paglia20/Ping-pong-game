/*
 * GccApplication1.c
 * Created: 02.09.2025
 * Author: Paglia20
 */

#define F_CPU 4915200UL  // 4.9152 MHz
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "include/UART.h"
#include "include/SRAM.h"
#include "include/decoder.h"
#include "include/ADC.h"
#include "include/joystick.h"
#include "include/slider.h"
#include "include/bit_macros.h"

//for part 4 Defines PD2 Buttons
void init_button(void) {
	clear_bit(DDRD,PD2); 
}

void pulse_ALE(void) {
    // Genera un impulso basso-alto-basso su PE1 per latchare i dati
    PORTE |= (1 << PE1);   // PE1 alto (latch abilita)
    _delay_us(1);
    PORTE &= ~(1 << PE1);  // PE1 basso (latch disabilita)
    _delay_us(1);
}

void test_dlatch(void){
    printf("Starting Latch test...\n\r");

     // Imposta PA0–PA7 come outpuADCt (bus dati)
    DDRA = 0xFF;
    // Imposta PE1 come output (ALE)
    DDRE |= (1 << PE1);

    // Inizializza uscite a zero
    PORTA = 0x00;
    PORTE &= ~(1 << PE1);

    uint8_t value = 0;

    while (1) {
        PORTA = value;     
        pulse_ALE();       
        _delay_ms(200);    
        value++;           
    } 
}

void test_uart(void) {
    printf("UART ready! Type something:\n\r");
    while (1) {
        char c = uart_getc();     // wait for user input (blocking so dots)
        //printf("typed c: %c\n\r", c, c);

        //uart_putc(c);             // echo it back
        printf("You typed: %c\n\r", c);        
    } 
}

void test_adc(void) {

    printf("ADC test start\r\n");

    while (1) {
        uint8_t *values = adc_read();  // CH0..CH3

        printf("X =%3u  Y =%3u  SLIDER =%3u  CH3 (/) =%3u\r\n",
               values[0], values[1], values[2], 0);

        _delay_ms(200);
    }
}

void test_joystick(void) {
    calibrate();

    printf("Joystick test start\r\n");

    print_zeros();

    while (1) {
        update_position();
        print_joystick();
        _delay_ms(200);
    }
} 

void test_slider(void) {
    calibrate_slider();

    printf("Slider test start\r\n");

    print_slider_zeros();

    while (1) {
        update_slider();
        print_slider();
        _delay_ms(200);
    }
} 

int main(void) {
    fflush(stdout);

   // INITS
    UART_init(F_CPU, BAUD);    // 9600 8N1
    XMEM_init();

    //test_uart();


    //TEST D LATCH
    //test_dlatch();

    //TEST SRAM

    //dec_test();
	//print_joystick();
    // SRAM_test(); 



    // // Test ADC
    adc_init();

    //test_adc();
    test_joystick();
    //test_slider();
}

