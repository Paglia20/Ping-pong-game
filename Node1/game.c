#include "../include/game.h"

volatile bool life;
volatile char score[50];

void start_game(void){
    run_menu = 0;
    life = 1;
    int iscore = 0;
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

        
        sprintf(score, "%10d", iscore); 


        if (CAN_receive(&rx)) {
            if ((rx.id == 0x03 && rx.dlc == 1 && rx.data[0] == 0x01) && life) {
                printf("Goal scored!\n\r");
                life = 0;
                oled_clear();
            }
        } else {
            iscore += 1;
        }
        
        if (board_read_buttons(&board)) {
            if (board_buttons_any(&board)) {
                if (board.R6) {
                    run_menu = 1;
                    menu_init();
                }
            }
        }
        
    }
}