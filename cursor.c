#include "include/OLED.h"
#include "include/SPI.h"
#include "include/cursor.h"
#include "include/joystick.h"

#include <util/delay.h>

void cursor_game(void){
    calibrate();

    Pixel p;
    p.page = 0;
    p.col = 0;
    p.bit = 0;

    while (1)
    {
        render_pixel(p);
        _delay_ms(100);
        update_pixel(&p);
    }

}

void update_pixel(Pixel *p) {
    // Update bit based on joystick input
    update_position();
    if (joystick.dir == UP && p->bit < 7) {
        p->bit++;
    } else if (joystick.dir == DOWN && p->bit > 0) {
        p->bit--;
    } else if (joystick.dir == LEFT && p->col > 0) {
        p->col--;
    } else if (joystick.dir == RIGHT && p->col < 127) {
        p->col++;
    }
}

void render_pixel(Pixel p) {
    oled_clear();
    oled_set_cursor((uint8_t)p.page, (uint8_t)p.col);

    uint8_t bitmask = (1 << p.bit);

    oled_write_data(&bitmask, 1);
}