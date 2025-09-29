#ifndef OLED_H
#define OLED_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

void oled_write_cmd1(uint8_t c);
void oled_write_cmd2(uint8_t c, uint8_t a0);
void oled_write_cmd3(uint8_t c, uint8_t a0, uint8_t a1);

void oled_write_data(const uint8_t* p, uint16_t n);

void OLED_init(void);

void OLED_fill_strips(void);

#endif // OLED_H