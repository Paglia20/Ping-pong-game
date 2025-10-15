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

    //Uncomment after including uart above
    uart_init(F_CPU, 115200);
    printf("Hello World\n\r");

    // 125 kbps, must match Node 1
    uint32_t can_br = 0x00290165;  // lab sheet value
    can_init_def_tx_rx_mb(can_br);

    printf("CAN initialized.\n\r");

    CAN_MESSAGE rx_msg;

    while (1)
    {
        if (can_receive(&rx_msg, 1) == 0)
        {
            printf("RX ID=0x%03X LEN=%d DATA:", rx_msg.id, rx_msg.data_length);
            for (uint8_t i = 0; i < rx_msg.data_length; i++)
                printf(" %02X", rx_msg.data[i]);
            printf("\n\r");
        }
    }
    
}