
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

#endif // CURSOR_H