#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// BUSY pin 
#define ADC_BUSY_PINR  PIND
#define ADC_BUSY_BIT   PD4      


void adc_init(void);
uint8_t *adc_read(void);

#endif // ADC_H