#include <stdio.h>
#include <stdarg.h>
#include "sam/sam3x/include/sam.h"

/*
 * Remember to update the Makefile with the (relative) path to the uart.c file.
 * This starter code will not compile until the UART file has been included in the Makefile. 
 * If you get somewhat cryptic errors referencing functions such as _sbrk, 
 * _close_r, _write_r, _fstat etc, you have most likely not done that correctly.

 * If you get errors such as "arm-none-eabi-gcc: no such file", you may need to reinstall the arm gcc packages using
 * apt or your favorite package manager.
 */
#include "uart.h"
#include "can_controller.h"

#define FREQ 84000000
#define BAUD 250000



int16_t convert_to_signed(uint16_t val) {
    if (val <= 32767)
        return (int16_t)val;
    else
        return (int16_t)(val - 65536);
}

int main()
{   
    SystemInit();

    WDT->WDT_MR = WDT_MR_WDDIS; //Disable Watchdog Timer

    //set high servo pin
    PMC ->PMC_PCER0 = (1 << ID_PIOB); 
    PIOB ->PIO_PER |= PIO_PB12;
    PIOB ->PIO_OER |= PIO_PB12; 
    PIOB ->PIO_SODR |= PIO_PB12; 

    //Enablecan_cfg clock for test should be 84MHz
    PMC->PMC_PCER0 = (1u << ID_PIOB);  
    PIOA->PIO_PDR   = PIO_PDR_P26;     
    PIOA->PIO_ABSR |= PIO_ABSR_P26;    

    //PCK0 source = MCK, prescaler = 2 (PRES=CLK_2).
    PMC->PMC_PCK[0] = PMC_PCK_CSS_MCK | PMC_PCK_PRES_CLK_2;  // MCK/2 = 42 MHz
    PMC->PMC_SCER = PMC_SCER_PCK0;
    while ((PMC->PMC_SR & PMC_SR_PCKRDY0) == 0) {}

    
    uart_init(F_CPU, 115200);
    printf("Hello World\n\r");


    uint8_t BRP = (FREQ / (16*BAUD)) - 1; // Baud Rate Prescaler

    //spiega questi valori
    uint32_t reg = ((0<<24) | (BRP<<16) | (3<<12) | (1<<8) | (6<<4) | 5);
    // SMP = 0 single sampling
    // BRP = BRP (computed)
    // SJW = 3  → 4 TQ
    // PROPAG = 1 → 2 TQ
    // PHASE1 = 6 → 7 TQ
    // PHASE2 = 5 → 6 TQ


    
    can_init_def_tx_rx_mb(reg);

    printf("CAN initialized.\n\r");

    CAN_MESSAGE rx_msg;

    while (1)
    {
        if (can_receive(&rx_msg, 0) == 0) {
            printf("RX ID=0x%03X LEN=%d DATA (X , Y, Slider):", rx_msg.id, rx_msg.data_length);
            for (uint8_t i = 0; i < rx_msg.data_length; i++){
                int val = convert_to_signed(rx_msg.data[i]);
                if (val > 100){
                    val -= 256;
                }
                printf(" %03d", rx_msg.data[i]);
            }
            printf("\n\r");
        }
    }

}
