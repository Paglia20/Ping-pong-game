#include "include/MCP2515.h"
#include "include/SPI.h"
#include "include/CAN.h"
#include <avr/interrupt.h>   

static inline void id_to_regs(uint16_t id, uint8_t* sidh, uint8_t* sidl) {
    *sidh = (uint8_t)(id >> 3);
    *sidl = (uint8_t)((id & 0x07) << 5);   // standard frame, no extended
}
static inline uint16_t regs_to_id(uint8_t sidh, uint8_t sidl) {
    return (uint16_t)((sidh << 3) | (sidl >> 5));
}

void CAN_init_loopback_125k_4M9(void)
{
    MCP_init();                 // set CS/INT pins, init SPI  (no reset here)
    MCP_reset();                // now CS is an output → reset works

    // Bit-timing per ~122.5 kbps (20 TQ, SJW=1TQ)
    // TQ = 2*(BRP+1)/Fosc -> BRP=0 -> TQ≈408ns; 20*TQ≈8.16us => 122.5 kbps
    MCP_write(MCP_CNF1, 0x00);  // SJW=00 (1TQ), BRP=000000
    MCP_write(MCP_CNF2, 0xAF);  // BTLMODE=1, SAM=0, PHSEG1=5 (PS1=6), PRSEG=7 (Prop=8)
    MCP_write(MCP_CNF3, 0x04);  // PHSEG2=4 (PS2=5)

    MCP_write(MCP_RXB0CTRL, 0x60);
    MCP_write(MCP_RXB1CTRL, 0x60);

    MCP_clear_interrupt_flags(0xFF);               
    MCP_enable_interrupts(MCP_RX_INT | MCP_TX_INT);  // single write
    MCP_clear_interrupt_flags(0xFF);

    MCP_set_mode(MODE_LOOPBACK);
}

bool CAN_send(const CanFrame* f)
{
    if (!f || f->dlc > 8) return false;

    uint8_t sidh, sidl;
    id_to_regs(f->id, &sidh, &sidl);

    MCP_write(TXB0SIDH, sidh);
    MCP_write(TXB0SIDL, sidl);
    MCP_write(TXB0DLC,  f->dlc & 0x0F);

    for (uint8_t i = 0; i < f->dlc; i++) {
        MCP_write((uint8_t)(TXB0D0 + i), f->data[i]);
    }
    
    //require transm
    MCP_rts(0);  
    return true;
}


bool CAN_receive(CanFrame* out)
{
    if (!out) return false;

    // double check shouldnt be necessary
    uint8_t flags = MCP_get_interrupt_flags();
    if (!(flags & MCP_RX0IF))
        return false;

    uint8_t sidh = MCP_read(RXB0SIDH);
    uint8_t sidl = MCP_read(RXB0SIDL);
    out->id  = regs_to_id(sidh, sidl);

    uint8_t dlc = MCP_read(RXB0DLC) & 0x0F;
    out->dlc = dlc;

    for (uint8_t i = 0; i < dlc; i++) {
        out->data[i] = MCP_read((uint8_t)(RXB0D0 + i));
    }

    MCP_clear_interrupt_flags(MCP_RX0IF);
    return true;
}

ISR(INT1_vect)
{   
    uint8_t flags = MCP_get_interrupt_flags();

    if (flags & MCP_RX0IF) { //perchè?  MCP_RX0IF
        CanFrame rx;

        CAN_receive(&rx);

        //handle received frame

        if (DEBUG_CAN) {
            printf("Received frame!\n");
            printf("ID: 0x%03X, DLC: %u, DATA:", rx.id, rx.dlc);
            for (uint8_t i = 0; i < rx.dlc; i++)
                printf(" %02X", rx.data[i]);
            printf("\n");
        }
    }

    if (flags & MCP_TX0IF) {
        MCP_clear_interrupt_flags(MCP_TX0IF);
        // printf transmission complete 
    }

    // other flags?
}