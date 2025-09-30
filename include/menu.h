// menu.h
#ifndef MENU_H
#define MENU_H

#include <stdbool.h>   // per il tipo bool
#include <stdint.h>    // se ci servono tipi fissi (opzionale)

#define MENU_STR_LEN 20   // lunghezza massima per la stringa

typedef struct {
    bool active;                    // flag booleano
    char label[MENU_STR_LEN];       // stringa fissa
} MenuItem;

typedef struct {
    int page;                    // flag booleano
    ManuItem* items;            // array di MenuItem
} Menu;

#endif // MENU_H