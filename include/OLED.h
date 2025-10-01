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

void oled_write_data(const uint8_t *p, uint16_t n);

void OLED_init(void);

void OLED_fill_strips(void);

void oled_home(void);
void oled_set_cursor(uint8_t page, uint8_t col);
void oled_newline(void);
void oled_putchar(char c);
void oled_print(const char *s);

void oled_clear_line(uint8_t page);
void oled_clear(void);

static inline void cs_low(void);
static inline void cs_high(void);

static inline void dc_cmd(void);
static inline void dc_data(void);

#endif // OLED_H