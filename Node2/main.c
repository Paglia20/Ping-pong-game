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
    PMC ->PMC_PCER0 = (1 << ID_PIOB); 
    PIOB ->PIO_PER |= PIO_PB12;
    PIOB ->PIO_OER |= PIO_PB12; 
    PIOB ->PIO_SODR |= PIO_PB12; 

    //Enable clock for test should be 84MHz
    PMC->PMC_PCER0 = (1u << ID_PIOA);  // enable PIOA clock
    PIOA->PIO_PDR   = PIO_PDR_P1;      // disable PIO control -> peripheral takes pin
    PIOA->PIO_ABSR |= PIO_ABSR_P1;     // select Peripheral B for PA1 (B=1 => PCK0 on PA1)

    //PCK0 source = MCK, prescaler = 2 (PRES=CLK_2).
    PMC->PMC_PCK[0] = PMC_PCK_CSS_MCK | PMC_PCK_PRES_CLK_2;  // MCK/2 = 42 MHz
    PMC->PMC_SCER = PMC_SCER_PCK0;
    while ((PMC->PMC_SR & PMC_SR_PCKRDY0) == 0) {}

    
    uart_init(F_CPU, 115200);
    printf("Hello World\n\r");

    // 500 kbps at 84 MHz
    CanInit can_cfg = {
        .phase2 = 6,
        .propag = 1, //supponendo 20 cm di can
        .phase1 = 6,
        .sjw = 3,
        .brp = 10,
        .smp = 0, 
    };
    can_init(can_cfg, 0);

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
    }
    
}