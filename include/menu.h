#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>

#include "../include/OLED.h"
#include "../include/SPI.h"
#include "../include/menu.h"
#include "../include/joystick.h"  
#include "../include/cursor.h"
#include "../include/board.h"
#include "../include/game.h"
#include <util/delay.h>
#include <stdio.h> 

#define MENU_STR_LEN 20

extern int run_menu;
extern BoardButtons board;

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
