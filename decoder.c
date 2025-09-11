
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "include/UART.h"
#include "include/SRAM.h"




// simple write to an absolute address in data space
static inline void poke(uint16_t addr, uint8_t val) {
    volatile uint8_t *p = (volatile uint8_t*)addr;
    *p = val;
}


static void burst_range(uint16_t start, uint16_t end) {
    // Step every 0x20 to make the CS LED clearly visible across the region
    for (uint16_t a = start; a <= end; a += 0x20) {
        poke(a, 0xA5);
        _delay_ms(60);
    }
}

void dec_test(void){

    // 1) ADC region — your decoder should assert ADC CS here
    printf("Testing memory region ADC\n");
    burst_range(0x1000, 0x17FF);
    printf("ADC test done\n");

    // 3) SRAM region — your decoder should assert SRAM CS here
    printf("Testing memory region SRAM\n");

    burst_range(0x1800, 0x1FFF);
    printf("SRAM test done\n");

    _delay_ms(300);
}