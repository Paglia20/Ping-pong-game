/*
 * GccApplication1.c
 * Created: 02.09.2025
 * Author: Paglia20
 */

#define F_CPU 4915200UL // 4.9152 MHz
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "../include/UART.h"
#include "../include/SRAM.h"
#include "../include/decoder.h"
#include "../include/ADC.h"
#include "../include/joystick.h"
#include "../include/slider.h"
#include "../include/bit_macros.h"
#include "../include/SPI.h"
#include "../include/OLED.h"
#include "../include/cursor.h"
#include "../include/menu.h"
#include "../include/CAN.h"
#include "../include/MCP2515.h"
#include <avr/interrupt.h>


void pulse_ALE(void)
{
    // Genera un impulso basso-alto-basso su PE1 per latchare i dati
    PORTE |= (1 << PE1); // PE1 alto (latch abilita)
    _delay_us(1);
    PORTE &= ~(1 << PE1); // PE1 basso (latch disabilita)
    _delay_us(1);
}

void test_dlatch(void)
{
    //printf("Starting Latch test...\n\r");

    DDRA = 0xFF;
    DDRE |= (1 << PE1);

    PORTA = 0x00;
    PORTE &= ~(1 << PE1);

    uint8_t value = 0;

    while (1)
    {
        PORTA = value;
        pulse_ALE();
        _delay_ms(200);
        value++;
    }
}

void test_uart(void)
{
    // printf("UART ready! Type something:\n\r");
    while (1)
    {
        char c = uart_getc(); // wait for user input (blocking so dots)
        if (!DEBUG) return;
        printf("typed c: %c\n\r", c, c);

        uart_putc(c);             // echo it back
        printf("You typed: %c\n\r", c);
    }
}

void test_adc(void)
{

    // printf("ADC test start\r\n");

    while (1)
    {
        uint8_t *values = adc_read(); // CH0..CH3
        if (!DEBUG) return;

        printf("X =%3u  Y =%3u  SLIDER =%3u",
               values[0], values[1], values[2]);

        _delay_ms(200);
    }
}

void test_joystick(void)
{
    calibrate();

    //printf("Joystick test start\r\n");

    print_zeros();

    while (1)
    {
        update_joystick();
        print_joystick();
        _delay_ms(200);
    }
}

void test_slider(void)
{
    calibrate_slider();

    // printf("Slider test start\r\n");

    print_slider_zeros();

    while (1)
    {
        update_slider();
        print_slider();
        _delay_ms(200);
    }
}

void test_OLED(void){
    // printf_P(PSTR("starting oled test\n\r"));
    
    _delay_ms(2000);

    // oled_clear();
    // oled_home();

    OLED_fill_strips();

    oled_print("HELLO WORLD");

    _delay_ms(2000);

    // oled_clear_line(0);

    oled_set_cursor(3,0);
    oled_print("Line 4 after clear");


    _delay_ms(2000);

    // oled_clear();
    // oled_home();
}

void test_loop(void){
    if (DEBUG_CAN) {
        printf("MCP2515 loopback  start...\n");
    }

    CAN_init_loopback();
    
    CanFrame tx = {
        .id  = 0x123,
        .dlc = 3,
        .data = { 0x11, 0x22, 0x33 }
    };

    //printf("Sending CAN frame...\n");
    CAN_send(&tx);

    CanFrame rx;
    while (!CAN_receive(&rx)) {
        // busy-wait
    }

    if (DEBUG_CAN) {
        printf("Received frame!\n");
        printf("ID: 0x%03X, DLC: %u, DATA:", rx.id, rx.dlc);
        for (uint8_t i = 0; i < rx.dlc; i++)
            printf(" %02X", rx.data[i]);
        printf("\n");
    }

}

void test_loop_int(void){
    if (DEBUG_CAN) {
        printf("MCP2515 loopback with int...\n");
    }

    CAN_init_loopback();


    MCUCR = (MCUCR & ~((1<<ISC11)|(1<<ISC10))) | (1<<ISC11);  // falling edge
    GICR  |= (1<<INT1);
    sei();

    uint8_t counter = 0;

    while (1){
        CanFrame tx = {
            .id  = 0x123,
            .dlc = 3,
            .data = { 0x11, 0x60, counter }
        };
    
        CAN_send(&tx);
        counter++;

        _delay_ms(1000);
    }
}

int main(void)
{

    fflush(stdout);
    // INITS
    UART_init(F_CPU, BAUD); // 9600 8N1
    XMEM_init();

    // printf_P(PSTR("XMEM init done\n\r"));
    // test_uart();

    // TEST D LATCH
    // test_dlatch();

    // TEST SRAM

    // dec_test();
    // print_joystick();
    //  SRAM_test();

    // // Test ADC
    adc_init();

    //test_adc();
    //test_joystick();
    // test_slider();

    SPI_init();

    OLED_init();

    //test_OLED();


    //cursor_game();

    //menu_init();

    //test_loop();

    //test_loop_int();
  
    // test_cs();


    test_nodes_communication();

    //send_joystick_data_over_can();


    return 0;   
}


void test_nodes_communication(void){
    if (DEBUG_CAN) {
        printf("MCP2515 normal mode start...\n");
    }

    CAN_init_normal_16TQ();

    uint8_t counter = 0;

    int run = 1;

    while (1){
        CanFrame tx = {
            .id  = 0x123,
            .dlc = 3,
            .data = { 0x41, 0x60, counter }
        };
    
        printf("Sending CAN frame...\n");
        CAN_send(&tx);
        counter++;
        printf("Counter: %X\n", counter);

        _delay_ms(1000);
        run = 0;

    }
}

void test_cs(void){
    SPI_init();
    while (1){
    SPI_select(SPI_SLAVE_CONTR);
    _delay_ms(1000);
    SPI_deselect(SPI_SLAVE_CONTR);
    _delay_ms(1000);
    SPI_select(SPI_SLAVE_IO);
    _delay_ms(1000);
    SPI_deselect(SPI_SLAVE_IO);
    _delay_ms(1000);
    SPI_select(SPI_SLAVE_OLED);
    _delay_ms(1000);
    SPI_deselect(SPI_SLAVE_OLED);
    _delay_ms(1000);
    }
}


void send_joystick_data_over_can(void){
    calibrate();
    calibrate_slider();

    CAN_init_normal_16TQ();

    while (1)
    {
        update_joystick();
        update_slider();

        uint8_t joy_x = joystick.x_val;
        uint8_t joy_y = joystick.y_val;
        uint8_t slider_x= slider.x_val;

        CanFrame tx = {
            .id  = 0x111,
            .dlc = 3,
            .data = {joy_x, joy_y, slider_x}
        };

        CAN_send(&tx);

        _delay_ms(100);
    }
}