#include "../include/game.h"


extern volatile u_int8_t lifes;
extern volatile char score[50];

void start_game(void){
    run_menu = 0;
    lifes = 0;
    float fscore = 0;
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
        //update_slider();
        oled_play();


        uint8_t dir = encode_direction(joystick.dir);

        CanFrame tx = {
            .id  = 0x111,
            .dlc = 4,
            .data = {dir, joystick.x_val_perc, joystick.y_val_perc, (uint8_t) joystick.button}
        };


        // printf("Sending direction:code: %02X)\n\r", dir);

        CAN_send(&tx);

        _delay_ms(100);

        fscore += 0.1;
        
        sprintf(score, "%.2f", fscore); 


        if (CAN_receive(&rx)) {
            if (rx.id == 0x03 && rx.dlc == 1 && rx.data[0] == 0x01) {
                printf("Goal scored!\n\r");
                lifes++;
            }
        }
    }
}