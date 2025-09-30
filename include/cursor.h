
#pragma once
#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>
#include <stdbool.h>   

typedef struct {
    int page;
    int col;    
    int bit;
} Pixel;

void cursor_game(void);
void update_pixel(Pixel *p);
void render_pixel(Pixel p);

#endif // CURSOR_H