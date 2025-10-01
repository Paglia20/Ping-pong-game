#pragma once
#ifndef SRAM_H
#define SRAM_H

#include <stdint.h>

void XMEM_init(void);

int SRAM_write(uint16_t address, char data);

uint8_t SRAM_read(uint16_t address);

void SRAM_test(void);


#endif // SRAM_H