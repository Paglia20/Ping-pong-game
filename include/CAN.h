#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "bit_macros.h"


// ---- Indirizzi che ci servono (coerenti con la mappa MCP2515) ----
#define TXB0SIDH  0x31
#define TXB0SIDL  0x32
#define TXB0DLC   0x35
#define TXB0D0    0x36

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
void CAN_init_loopback_125k_4M9(void);

// send one frame using TXB0
bool CAN_send(const CanFrame* f);

// receive one frame from RXB0 (returns false if nothing)
bool CAN_receive(CanFrame* out);

void CAN_init_normal_500k_16(void);




