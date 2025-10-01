#include "include/OLED.h"
#include "include/SPI.h"
#include "include/menu.h"
#include "include/joystick.h"  
#include "include/cursor.h"
#include <util/delay.h>
#include <stdio.h> 


void test_action(void);

// Keep track of current and parent menu (for "Back")
static Menu* currentMenu = NULL;
static Menu  subMenuObj;      
static Menu* parentMenu = NULL;
static uint8_t selectedIndex = 0;


MenuItem submenu_items[3] = {
    {true, "Sub Item 1", NULL, test_action},
    {true, "Sub Item 2", NULL, test_action},
    {true, "Back",       NULL, back_action}
};

MenuItem main_menu_items[3] = {
    {true, "Start",    NULL, cursor_game},
    {true, "Settings", submenu_items, NULL},
    {true, "About",    NULL,  test_action}
};

Menu mainMenu = {
    .page = 0,
    .items = main_menu_items
};

void menu_init(void){
    SPI_init();
    OLED_init();
    calibrate();  

    currentMenu = &mainMenu;
    selectedIndex = 0;
    draw_menu(currentMenu);

    while (1) {
        menu_loop();
    }
}

// === MENU DRAW ===
void draw_menu(Menu* menu) {
    oled_clear();

    for (int i = 0; i < 3; i++) {
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
    parentMenu = currentMenu;      // remember where we came from
    subMenuObj.items = subMenu;    // wrap the raw items array into a Menu object
    subMenuObj.page  = 0;

    currentMenu = &subMenuObj;
    selectedIndex = 0;
    draw_menu(currentMenu);
}

void execute_action(MenuItem* item) {
    // Example: show the label temporarily
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
    if (selectedIndex < 2) {  // You can make this dynamic later
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
