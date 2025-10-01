#include "include/OLED.h"
#include "include/SPI.h"
#include "include/menu.h"
#include "include/joystick.h"  // ⬅ Needed for joystick.dir
#include <util/delay.h>
#include <stdio.h> // For printf

// === Global Menu ===
// ⚠️ Define your menu items here (or in a separate file if preferred)
MenuItem submenu_items[] = {
    {true, "Sub Item 1", NULL},
    {true, "Sub Item 2", NULL},
    {true, "Back", NULL}
};

MenuItem main_menu_items[] = {
    {true, "Start", NULL},
    {true, "Settings", submenu_items},
    {true, "About", NULL}
};

Menu mainMenu = {
    .page = 0,
    .items = main_menu_items
};

static uint8_t selectedIndex = 0;
static Menu* currentMenu = &mainMenu;  // 🧠 Track which menu we're in

// === INIT ===
void menu_init(void){
    SPI_init();
    OLED_init();
    calibrate();  // if joystick needs calibration

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
            oled_set_cursor(i, 8);  // shift text to right (cursor at 0)
            oled_print(menu->items[i].label);
        }
    }

    // Draw selector arrow ">"
    oled_set_cursor(selectedIndex, 0);
    oled_putchar('>');
}

// === SUBMENU DRAW ===
void draw_submenu(MenuItem* subMenu) {
    currentMenu = (Menu*)malloc(sizeof(Menu));
    currentMenu->items = subMenu;
    currentMenu->page = 0;
    selectedIndex = 0;
    draw_menu(currentMenu);
}

// === EXECUTE ACTION ===
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
    } else {
        execute_action(selected_item);
    }
}


// === MAIN LOOP ===
void menu_loop(void) {
    update_position(); // ⬅️ Update joystick status
    Direction d = joystick.dir;

    if (d == UP) {
        menu_navigation_up();
    } else if (d == DOWN) {
        menu_navigation_down();
    }

    if (joystick.button_pressed) {  // ⬅️ This assumes your struct has this field
        menu_select();

        // Wait for button release to prevent double-trigger
        while (joystick.button_pressed) {
            update_position();
        }
    }

    _delay_ms(120); // Debounce and limit update rate
}
