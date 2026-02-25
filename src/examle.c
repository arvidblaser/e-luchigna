#include <zephyr/kernel.h>
#include <nrfx_pwm.h>
#include <hal/nrf_gpio.h>

#define COM_PIN      17
#define SEG_1_PIN    12  
#define SEG_2_PIN    13  
#define SEG_3_PIN    14  
#define SEG_4_PIN    15  
#define SEG_5_PIN    16  
#define SEG_6_PIN    18  
#define SEG_7_PIN    19  


uint8_t SEGMENTS[7] = {SEG_1_PIN, SEG_2_PIN, SEG_3_PIN, SEG_4_PIN, SEG_5_PIN, SEG_6_PIN, SEG_7_PIN};    

// Not used yet, but will be useful in the future to translate digits to segments

uint8_t NUMBER_TO_SEGMENTS[17] = {

        0b00111111, // 0 - ABCDEF
        0b00000110, // 1 - BC
        0b01011011, // 2 - ABGED
        0b01001111, // 3 - ABGCD
        0b01100110, // 4 - FGBC
        0b01101101, // 5 - AFGCD
        0b01111101, // 6 - AFGECD
        0b00000111, // 7 - ABC
        0b01111111, // 8 - ABCDEFG
        0b01101111, // 9 - ABCDFG
        0b01110111, // A - ABCEFG
        0b01111100, // B - FGECD
        0b00111001, // C - AFED
        0b01011110, // D - BCDEG
        0b01111001, // E - AFGED
        0b01110001, // F - AFGE
        0b00000000, // OFF
};

void set_all_segments(bool active) {
        for (int i = 0; i < 7; i++) {
                nrf_gpio_cfg_output(SEGMENTS[i]);
                nrf_gpio_pin_write(SEGMENTS[i], active);
        }
}

void set_single_segment(uint8_t segment, bool active) {
        uint8_t seg = SEGMENTS[segment];
        nrf_gpio_cfg_output(seg);
        nrf_gpio_pin_write(seg, active);
}

void hold_all_segments(){
        for (int i = 0; i < 7; i++) {
                nrf_gpio_cfg_default(SEGMENTS[i]);
        }
}

void hold_single_segment(uint8_t segment) {
        uint8_t seg = SEGMENTS[segment];
        nrf_gpio_cfg_default(seg);
}

 

void main(void) {
        // Tie COM pin to GND, and it should not be SET anywhere
        nrf_gpio_cfg_output(COM_PIN);    
        nrf_gpio_pin_write(COM_PIN, 0);

        while (1) {
                // STEP 1: Activation Pulse (0V COM, 3.3V SEG) for 1.5s to drive color deep
                set_all_segments(true);
                k_msleep(1500);

                // STEP 2: Hold 3000ms
                hold_all_segments();
                k_msleep(3000);

                /* --- ACTION: OFF  --- */
                // Short both sides to 0V
                set_all_segments(false);
                k_msleep(3000);

                // show one segment at a time, from A to G
                for (int i = 0; i < 7; i++) {
                        // STEP 3: Activation Pulse (0V COM, 3.3V SEG) for 1.5s to drive color deep
                        set_single_segment(i, true);
                        k_msleep(1500);

                        // STEP 4: Hold 3000ms
                        hold_single_segment(i);
                        k_msleep(3000);

                        /* --- ACTION: OFF  --- */
                        // Short both sides to 0V
                        set_single_segment(i, false);
                        k_msleep(3000);
                }
        }
}