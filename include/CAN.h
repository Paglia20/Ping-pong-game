#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "bit_macros.h"


// ---- Indirizzi che ci servono (coerenti con la mappa MCP2515) ----
#define TXB1SIDH  0x41
#define TXB1SIDL  0x42
#define TXB1DLC   0x45
#define TXB1D0    0x46

#define RXB0SIDH  MCP_RXB0SIDH   
#define RXB0SIDL  (MCP_RXB0SIDH+1)
#define RXB0DLC   (MCP_RXB0SIDH+4)
#define RXB0D0    (MCP_RXB0SIDH+5)

typedef struct {
    uint16_t id;      // 11-bit standard ID
    uint8_t  dlc;     // 0..8 how many data bytes the message carries.
    uint8_t  data[8];
} CanFrame;

// init: reset, bit-timing per ~125 kbps @ 4.9 MHz, accept-all, loopback
void CAN_init_loopback(void);

// send one frame using TXB0
bool CAN_send(const CanFrame* f);

// receive one frame from RXB0 (returns false if nothing)
bool CAN_receive(CanFrame* out);

void CAN_init_normal_16TQ(void);



