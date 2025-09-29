#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#define OLED_DC_PORT PORTD
#define OLED_DC_DDR  DDRD
#define OLED_DC_PIN  PD2   

void oled_write_cmd1(uint8_t c);

void oled_write_cmd2(uint8_t c, uint8_t a0);
void oled_write_cmd3(uint8_t c, uint8_t a0, uint8_t a1);

void oled_write_data(const uint8_t* p, uint16_t n);


void OLED_init(void);

void OLED_fill(uint8_t pattern);