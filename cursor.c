#include <stdio.h>
#include <util/delay.h>
#include "include/OLED.h"
#include "include/SPI.h"
#include "include/cursor.h"
#include "include/joystick.h"

static inline void write_byte(uint8_t page, uint8_t col, uint8_t value) {
    oled_set_cursor(page, col);           // set page + column
    oled_write_data(&value, 1);           // write exactly one byte
}

static inline void draw_dot(const Pixel* p) {
    uint8_t mask = (uint8_t)(1u << p->bit);
    write_byte(p->page, p->col, mask);
}

static inline void erase_dot(const Pixel* p) {
    write_byte(p->page, p->col, 0x00);
}

static inline int pixels_equal(const Pixel* a, const Pixel* b) {
    return (a->page == b->page) && (a->col == b->col) && (a->bit == b->bit);
}

static inline void clamp_pixel(Pixel* p) {
    if (p->page > 7)   p->page = 7;
    if (p->col  > 127) p->col  = 127;
    if (p->bit  > 7)   p->bit  = 7;
}

static void advance_pixel(Pixel* p, Direction dir) {
    if (dir == UP) {
        if (p->bit == 7) { if (p->page < 7) { p->bit = 0; p->page++; } }
        else p->bit++;
    } else if (dir == DOWN) {
        if (p->bit == 0) { if (p->page > 0) { p->bit = 7; p->page--; } }
        else p->bit--;
    } else if (dir == LEFT) {
        if (p->col > 0) p->col--;
    } else if (dir == RIGHT) {
        if (p->col < 127) p->col++;
    }
    clamp_pixel(p);
}

void cursor_game(void) {
    printf("Cursor game start!\r\n");

    SPI_init();
    OLED_init();      // make sure your init sets page addressing mode
    calibrate();      // joystick calibration

    oled_clear();     // clear once at start

    Pixel cur  = { .page = 0, .col = 0, .bit = 0 };
    Pixel prev = cur;
    draw_dot(&cur);

    while (1) {
        update_position();                     // updates global joystick.dir
        Direction d = joystick.dir;

        if (d != NEUTRAL) {
            prev = cur;
            advance_pixel(&cur, d);

            if (!pixels_equal(&prev, &cur)) {
                erase_dot(&prev);              // clear old location (one byte)
                draw_dot(&cur);                // draw new location (one byte)
            }
        }

        // Optional: debug
        // print_joystick();

        _delay_ms(10);                         // small, smooth cadence
    }
}