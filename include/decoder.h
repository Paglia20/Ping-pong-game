#pragma once
#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

void dec_test(void);
static void burst_range(uint16_t start, uint16_t end);
static inline void poke(uint16_t addr, uint8_t val);

#endif // DECODER_H