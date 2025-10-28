#include <stdio.h>
#include "sam/sam3x/include/sam.h"

#include "main.h"

void PIOA_Handler(void){
    uint32_t isr = PIOA->PIO_ISR;      
    if (isr & PIO_ISR_P2){
        ball_count++;
    }
}