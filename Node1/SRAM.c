#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>

#include "../include/bit_macros.h"
#include "../include/SRAM.h"


#define FIRST_ADDRESS 0x1800
volatile char *ext_ram = (char *) FIRST_ADDRESS; 


void XMEM_init(void){

	set_bit(MCUCR,SRE);		//Enable External Memory Interface. 
	set_bit(SFIOR,XMM2);	//Remove 4 MSB for JTAG
}


int SRAM_write(uint16_t address, char data){
	if (address <= 0x7FF){
        ext_ram[address] = data;
        return 0;
		
	}
    //printf("Trying to write a too big address\n");
	return EXIT_FAILURE;
}

uint8_t SRAM_read(uint16_t address){
    if (address <= 0x7FF){
        return ext_ram[address];
    }
    //printf("Trying to read a too big address\n");
    return EXIT_FAILURE;
}


void SRAM_test(void)
{
    uint16_t ext_ram_size = 0x800;
    uint16_t write_errors = 0;
    uint16_t retrieval_errors = 0;
    //printf("Starting SRAM test...\n");

    // rand() stores some internal state, so calling this function in a loop
    // will yield different seeds each time (unless srand() is called before this function)
    uint16_t seed = rand();

    // Write phase: Immediately check that the correct value was stored
    srand(seed);
    for (uint16_t i = 0; i < ext_ram_size; i++) {
        uint8_t some_value = rand();
        ext_ram[i] = some_value;
        uint8_t retrieved_value = ext_ram[i];
        if (retrieved_value != some_value) {
            //printf("Write phase error: ext_ram[%4d] = %02X (should be %02X)\n", i, retrieved_value, some_value);
            write_errors++;
        }
    }

    // Retrieval phase: Check that no values were changed during or after the write phase
    srand(seed); // reset the PRNG to the state it had before the write phase
    for (uint16_t i = 0; i < ext_ram_size; i++) {
        uint8_t some_value = rand();
        uint8_t retrieved_value = ext_ram[i];
        if (retrieved_value != some_value) {
            //printf("Retrieval phase error: ext_ram[%4d] = %02X (should be %02X)\n", i, retrieved_value, some_value);
            retrieval_errors++;
        }
    }

    //printf("SRAM test completed with \n%4d errors in write phase and \n%4d errors in retrieval phase\n\n", write_errors, retrieval_errors);
}

// End of SRAM.c