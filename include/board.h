#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct BoardButtons {

    uint8_t R1:1;
    uint8_t R2:1;
    uint8_t R3:1;
    uint8_t R4:1;
    uint8_t R5:1;
    uint8_t R6:1;

    uint8_t L1:1;
    uint8_t L2:1;
    uint8_t L3:1;
    uint8_t L4:1;
    uint8_t L5:1;
    uint8_t L6:1;
    uint8_t L7:1;

    uint8_t Up:1;
    uint8_t Down:1;
    uint8_t Left:1;
    uint8_t Right:1;
    uint8_t Btn:1;
} BoardButtons;

BoardButtons board_init(void);

bool board_read_buttons(BoardButtons* out);

bool board_read_buttons_raw(uint8_t bytes3_out[3]);

static inline bool board_buttons_any(const BoardButtons* b) {
    return (b->R1|b->R2|b->R3|b->R4|b->R5|b->R6|
            b->L1|b->L2|b->L3|b->L4|b->L5|b->L6|b->L7|
            b->Up|b->Down|b->Left|b->Right|b->Btn) != 0;
}

bool board_set_led(uint8_t led, bool on);
bool board_sed_pwm(uint8_t led, uint8_t pwd);
bool read_info(void);

#endif // BOARD_H