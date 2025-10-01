
#pragma once
#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>
#include <stdbool.h>   

typedef struct {
    uint8_t page; // 0..7 (bande da 8 px)
    uint8_t col;  // 0..127
    uint8_t bit;  // 0..7 (offset verticale dentro la page)
} Sprite8;

void cursor_game(void);

#endif // CURSOR_H