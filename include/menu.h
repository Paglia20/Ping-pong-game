#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>

#define MENU_STR_LEN 20

extern int run_menu;

typedef void (*MenuAction)(void);

typedef struct MenuItem {
    bool active;
    char label[MENU_STR_LEN];
    struct MenuItem* sub;  
    MenuAction action;      
} MenuItem;

typedef struct {
    uint8_t count;
    MenuItem* items;
} Menu;

void menu_init(void);
void menu_loop(void);

#endif // MENU_H
