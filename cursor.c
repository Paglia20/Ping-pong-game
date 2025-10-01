#include <string.h>
#include <stdio.h>
#include <util/delay.h>

#include "include/OLED.h"
#include "include/SPI.h"
#include "include/cursor.h"
#include "include/joystick.h"

// --------- Framebuffer (shadow) ----------
static uint8_t fb[8][128];

// --------- Low-level byte writes ----------
static inline void write_byte(uint8_t page, uint8_t col) {
    oled_set_cursor(page, col);          // set page + column
    oled_write_data(&fb[page][col], 1);  // write exactly one byte
}

static inline void set_pixel_bit(const Pixel* p) {
    fb[p->page][p->col] |= (uint8_t)(1u << p->bit);
    write_byte(p->page, p->col);
}

static inline void clear_pixel_bit(const Pixel* p) {
    fb[p->page][p->col] &= (uint8_t)~(1u << p->bit);
    write_byte(p->page, p->col);
}

// --------- Helpers ----------
static inline void fb_clear_all(void) {
    memset(fb, 0, sizeof(fb));
}

static inline bool pixel_equal(const Pixel* a, const Pixel* b) {
    return (a->page == b->page) && (a->col == b->col) && (a->bit == b->bit);
}

static inline void clamp_pixel(Pixel* p) {
    if (p->page > 7) p->page = 7;
    if (p->col  > 127) p->col = 127;
    if (p->bit  > 7) p->bit = 7;
}

// Advance the pixel by one “step” based on the current joystick direction.
static void advance_pixel(Pixel* p, JoystickDirection dir) {
    // vertical movement: change bit inside the same page byte,
    // and roll to next/prev page when passing edge
    if (dir == UP) {
        if (p->bit == 7) {
            if (p->page < 7) { p->bit = 0; p->page++; }
        } else {
            p->bit++;
        }
    } else if (dir == DOWN) {
        if (p->bit == 0) {
            if (p->page > 0) { p->bit = 7; p->page--; }
        } else {
            p->bit--;
        }
    } else if (dir == LEFT) {
        if (p->col > 0) p->col--;
    } else if (dir == RIGHT) {
        if (p->col < 127) p->col++;
    }
    clamp_pixel(p);
}

// Draw only what changed (erase old bit, set new bit).
static inline void draw_move(const Pixel* prev, const Pixel* cur) {
    if (!pixel_equal(prev, cur)) {
        clear_pixel_bit(prev);
        set_pixel_bit(cur);
    }
}

// Optional: simple movement throttling so it doesn’t move too fast when held.
// Returns true if we should take a step on this loop.
static bool should_step(void) {
    // crude throttle: ~8-10ms delay already present; you can add a counter here
    // if you want slower motion. For now always step if a direction is pressed.
    return true;
}

// --------- Public entry point ----------
void cursor_game(void) {
    printf("Cursor game start!\r\n");

    SPI_init();
    OLED_init();         // make sure init sets page addressing and cursor funcs
    calibrate();         // joystick calibration (your existing function)

    fb_clear_all();
    oled_clear();        // clear panel once

    Pixel cur  = { .page = 0, .col = 0, .bit = 0 };
    Pixel prev = cur;

    set_pixel_bit(&cur); // draw first dot

    while (1) {
        update_position();          // updates global `joystick.dir`
        JoystickDirection d = joystick.dir;

        if (d != NEUTRAL && should_step()) {
            prev = cur;
            advance_pixel(&cur, d);
            draw_move(&prev, &cur);
        }

        // optional debug
        // print_joystick();

        _delay_ms(10);   // small delay to avoid busy-spin & keep motion smooth
    }
}