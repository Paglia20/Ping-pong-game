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
#include "can.h"

int main()
{   
    SystemInit();

    WDT->WDT_MR = WDT_MR_WDDIS; //Disable Watchdog Timer

    //set high servo pin
    PMC ->PMC_PCER0 |= (1 << ID_PIOB); 
    PIOB ->PIO_PER |= PIO_PB12;
    PIOB ->PIO_OER |= PIO_PB12; 
    PIOB ->PIO_SODR |= PIO_PB12; 

    // //clock out
    PMC->PMC_PCER0 |= (1 << ID_PIOB);

    // Configure PCK0: source = Master Clock (MCK), no prescaler
    PMC->PMC_PCK[0] = PMC_PCK_CSS_MCK | PMC_PCK_PRES(0);
    PMC->PMC_SCER |= PMC_SCER_PCK0; // Enable PCK0

    // Wait for the clock to be ready
    while (!(PMC->PMC_SR & PMC_SR_PCKRDY0));

    // --- Route PCK0 to PB27 (Arduino pin 13) ---
    // Peripheral B function on PB27 → PCK0
    const uint32_t PB27 = (1u << 27);  // Bit mask for PB27

    // Select peripheral B for PB27 (set bit in ABSR)
    PIOB->PIO_ABSR |= PB27;

    // Disable PIO control so peripheral can drive the pin
    PIOB->PIO_PDR  |= PB27;


    //Uncomment after including uart above
    uart_init(F_CPU, 115200);
    printf("Hello World\n\r");

    // 125 kbps, must match Node 1
    uint32_t can_br = 0x00053255;  // lab sheet value
    CanInit cfg;
    cfg.reg = can_br;
    can_init(cfg, 0);

    printf("CAN initialized.\n\r");

    CanMsg rx_msg;

    while (1)
    {
        if (can_rx(&rx_msg) == 1) {
            printf("RX ID=0x%03X LEN=%d DATA:", rx_msg.id, rx_msg.length);
            for (uint8_t i = 0; i < rx_msg.length; i++){
                printf(" %02X", rx_msg.byte[i]);
            }
            printf("\n\r");
        }
        //_delay_ms(1000);

    }
    
}