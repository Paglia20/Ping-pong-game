
#include <OLED.h>
#include <SPI.h>

static inline void oled_cmd(uint8_t c) {
    OLED_DC_PORT &= ~(1<<OLED_DC_PIN);  // D/C = 0 -> command
    SPI_txrx(c);
}
static inline void oled_data(uint8_t d) {
    OLED_DC_PORT |= (1<<OLED_DC_PIN);   // D/C = 1 -> data
    SPI_txrx(d);
}
