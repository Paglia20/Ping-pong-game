#include "include/OLED.h"
#include "include/SPI.h"
#include "include/menu.h"
#include "include/joystick.h"  
#include "include/cursor.h"
#include "include/board.h"
#include <util/delay.h>
#include <stdio.h> 

int run = 1;
BoardButtons board;

void test_action(void);
void back_action(void);
void power_off(void);
void test_avr(void);

// Keep track of current and parent menu (for "Back")
static Menu* currentMenu = NULL;
static Menu  subMenuObj;      
static Menu* parentMenu = NULL;
static uint8_t selectedIndex = 0;


MenuItem submenu_items[3] = {
    {true, "Calibrate", NULL, calibrate},
    {true, "Test AVR", NULL, test_avr},
    {true, "Back",       NULL, back_action}
};

MenuItem main_menu_items[4] = {
    {true, "Start",    NULL, cursor_game},
    {true, "Settings & Tests", submenu_items, NULL},
    {true, "About",    NULL,  test_action},
    {true, "Power OFF",    NULL,  power_off}
};

Menu mainMenu = {
    .count = sizeof(main_menu_items) / sizeof(main_menu_items[0]),
    .items = main_menu_items
};

void menu_init(void){
    SPI_init();
    OLED_init();
    board = board_init();

    calibrate();  

    currentMenu = &mainMenu;
    selectedIndex = 0;
    draw_menu(currentMenu);

    while (run) {
        menu_loop();
    }
}

// === MENU DRAW ===
void draw_menu(Menu* menu) {
    oled_clear();

    for (int i = 0; i < menu->count; i++) {
        if (menu->items[i].active) {
            oled_set_cursor(i, 8); 
            oled_print(menu->items[i].label);
        }
    }

    oled_set_cursor(selectedIndex, 0);
    oled_putchar('>');
}

// === SUBMENU DRAW ===
void draw_submenu(MenuItem* subMenu) {
    parentMenu = currentMenu;     
    subMenuObj.items = subMenu;   
    subMenuObj.count  = sizeof(submenu_items) / sizeof(submenu_items[0]);

    currentMenu = &subMenuObj;
    selectedIndex = 0;
    draw_menu(currentMenu);
}

void execute_action(MenuItem* item) {
    oled_clear();
    oled_set_cursor(3, 10);
    oled_print("Selected:");
    oled_set_cursor(4, 10);
    oled_print(item->label);

    _delay_ms(1000);
    draw_menu(currentMenu);
}

// === NAVIGATION ===
void menu_navigation_up(void) {
    if (selectedIndex > 0) {
        selectedIndex--;
        draw_menu(currentMenu);
    }
}

void menu_navigation_down(void) {
    if (selectedIndex < (currentMenu->count - 1)) {  
        selectedIndex++;
        draw_menu(currentMenu);
    }
}

// === SELECT ===
void menu_select(void) {
    MenuItem* selected_item = &currentMenu->items[selectedIndex];

    if (selected_item->sub != NULL) {
        draw_submenu(selected_item->sub);
        return;
    }

    if (selected_item->action) {
        selected_item->action();   // run the callback
        return;
    }

    // Fallback: generic action display
    execute_action(selected_item);
}


// === MAIN LOOP ===
void menu_loop(void) {
    update_joystick(); 
    Direction d = joystick.dir;

    if (d == UP) {
        menu_navigation_up();
    } else if (d == DOWN) {
        menu_navigation_down();
    }

    if (joystick.button) { 
        menu_select();

        while (joystick.button) {
            update_joystick();
        }
    }

    _delay_ms(120);
}


void test_action(void){
    oled_clear();
    oled_set_cursor(3, 10);
    oled_print("TEST ACTION..");
    _delay_ms(800);
    draw_menu(currentMenu);
}


void back_action(void){
    if (parentMenu) {
        currentMenu = parentMenu;
        parentMenu = NULL;
    } else {
        currentMenu = &mainMenu;
    }
    selectedIndex = 0;
    draw_menu(currentMenu);
}

void power_off(void){
    currentMenu = &mainMenu;
    selectedIndex = 0;
    run = 0;
}

void test_avr(void){
    oled_clear();
    oled_set_cursor(2, 0);
    oled_print("Press any button.. (R1 to exit)");
    int test = 1;
    
    while (test) {
        oled_set_cursor(3, 0);
        oled_print("Last pressed:");

        if (board_read_buttons(&board)) {
            if (board_buttons_any(&board)) {
                oled_clear_line(3);
                oled_print("Last pressed:");

                if (board.Btn)   oled_print("NAV btn");
                if (board.Up)    oled_print("NAV up");
                if (board.Down)  oled_print("NAV down");
                if (board.Left)  oled_print("NAV left");
                if (board.Right) oled_print("NAV right");
                if (board.L1)    {oled_print("L1"); board_set_led(0, true);}                //test turn on led from menu
                if (board.L2)    {oled_print("L2"); board_set_led(0, false);}

                if (board.R1)    {test = 0;}
            }
        }

        //not sure if the behaviour will be correct
        _delay_ms(10);
    }

    draw_menu(currentMenu);

}

