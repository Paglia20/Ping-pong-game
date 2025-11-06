#include "../include/game.h"

void start_game(void){
    run_menu = 0;
    oled_clear();

    calibrate();
    calibrate_slider();

    CAN_init_normal_16TQ();

    //start transmission with 0x100
    CanFrame tx = {
           .id  = 0x100,
           .dlc = 1,
           .data = {0x00}
    };

    CAN_send(&tx);

    _delay_ms(100);

    CanFrame rx;

    while (!run_menu)
    {
        update_joystick();
        update_slider();


        uint8_t dir = encode_direction(joystick.dir);

        CanFrame tx = {
            .id  = 0x111,
            .dlc = 1,
            .data = {dir}
        };


        // printf("Sending direction:code: %02X)\n\r", dir);

        CAN_send(&tx);

        _delay_ms(100);


        if (CAN_receive(&rx)) {
            if (rx.id == 0x200 && rx.dlc == 1 && rx.data[0] == 0x01) {
                printf("Goal scored! Exiting game loop.\n\r");
                run_menu = 1;
            }
        }
    }
}