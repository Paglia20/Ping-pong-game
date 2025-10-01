#include <stdio.h>
#include <util/delay.h>

#include "include/OLED.h"
#include "include/SPI.h"
#include "include/cursor.h"
#include "include/joystick.h"

static inline void write_byte(uint8_t page, uint8_t col, uint8_t value) {
    oled_set_cursor(page, col);
    oled_write_data(&value, 1);
}


static inline void write_two_bytes(uint8_t page, uint8_t col, uint8_t low, uint8_t high) {
    oled_set_cursor(page, col);
    oled_write_data(&low, 1);
    oled_set_cursor((uint8_t)(page + 1), col);
    oled_write_data(&high, 1);
}

static inline void draw_sprite8(const Sprite8* s) {
    const uint8_t shift = s->bit & 7;

    if (shift == 0) {
        for (uint8_t dx = 0; dx < 8; ++dx) {
            write_byte(s->page, (uint8_t)(s->col + dx), 0xFF);
        }
    } else {
        const uint8_t low  = (uint8_t)(0xFFu << shift);         
        const uint8_t high = (uint8_t)(0xFFu >> (8 - shift));   
        for (uint8_t dx = 0; dx < 8; ++dx) {
            write_two_bytes(s->page, (uint8_t)(s->col + dx), low, high);
        }
    }
}

static inline void erase_sprite8(const Sprite8* s) {
    const uint8_t shift = s->bit & 7;

    if (shift == 0) {
        for (uint8_t dx = 0; dx < 8; ++dx) {
            write_byte(s->page, (uint8_t)(s->col + dx), 0x00);
        }
    } else {
        for (uint8_t dx = 0; dx < 8; ++dx) {
            write_two_bytes(s->page, (uint8_t)(s->col + dx), 0x00, 0x00);
        }
    }
}

static inline int sprites_equal(const Sprite8* a, const Sprite8* b) {
    return (a->page == b->page) && (a->col == b->col) && (a->bit == b->bit);
}


static inline void clamp_sprite(Sprite8* s) {
    if (s->col > 120) s->col = 120;

    if (s->bit == 0) {
        if (s->page > 7) s->page = 7;
    } else {
        if (s->page > 6) s->page = 6; // serve spazio per la page+1
    }
}

static void advance_sprite(Sprite8* s, Direction dir) {
    if (dir == DOWN) {
        if (s->bit == 7) {
            if (s->page < 7) { s->bit = 0; s->page++; }
        } else {
            s->bit++;
        }
    } else if (dir == UP) {
        if (s->bit == 0) {
            if (s->page > 0) { s->bit = 7; s->page--; }
        } else {
            s->bit--;
        }
    } else if (dir == LEFT) {
        if (s->col > 0) s->col--;
    } else if (dir == RIGHT) {
        if (s->col < 120) s->col++;
    }
    clamp_sprite(s);
}



void cursor_game(void) {
    printf("Cursor game start!\r\n");

    SPI_init();
    OLED_init();      
    calibrate();      

    OLED_fill_strips();
    _delay_ms(1000);

    oled_clear();

    Sprite8 cur  = { .page = 0, .col = 0, .bit = 0 };
    Sprite8 prev = cur;

    draw_sprite8(&cur);

    while (1) {
        update_position();
        Direction d = joystick.dir;

        if (d != NEUTRAL) {
            prev = cur;
            advance_sprite(&cur, d);

            if (!sprites_equal(&prev, &cur)) {
                erase_sprite8(&prev);
                draw_sprite8(&cur);
            }
        }

        // debug opzionale
        // print_joystick();

        _delay_ms(10);
    }
}

void test (void){
    printf("Cursor test start!\r\n");
    Sprite8 s = { .page = 3, .col = 40, .bit = 3 };
    draw_sprite8(&s);

    while (1) { _delay_ms(200); }
}

