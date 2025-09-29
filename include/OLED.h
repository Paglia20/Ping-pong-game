#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#define OLED_DC_PORT PORTD
#define OLED_DC_DDR  DDRD
#define OLED_DC_PIN  PD2   
