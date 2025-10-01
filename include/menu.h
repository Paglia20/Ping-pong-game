#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>

#define MENU_STR_LEN 20

// ✅ Forward declaration only needed if recursive
// typedef struct MenuItem MenuItem;  ← Not necessary here if we define full struct

typedef struct MenuItem {
    bool active;
    char label[MENU_STR_LEN];
    struct MenuItem* sub;  // ✅ correct: use struct MenuItem* here
} MenuItem;

typedef struct {
    int page;
    MenuItem* items;
} Menu;

// Function declarations
void draw_menu(Menu* menu);
void menu_navigation_up(void);
void menu_navigation_down(void);
void menu_select(void);
void draw_submenu(MenuItem* subMenu);
void execute_action(MenuItem* item);
void menu_loop(void);

#endif // MENU_H
