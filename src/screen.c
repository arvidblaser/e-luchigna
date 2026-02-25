#include "screen.h"

// delaytimerefresh
//delaytime turn on

//pins to turn on array
//pins to turn off array
//pinns to refresh array
// activePins

//function turnON(pinstoTurnon)
//function turnff(pinstoTurnoff)

//enum displaytype 1, 2

typedef uint8_t seg7_pattern_t;

enum {
    SEG7_ZERO  = 0b00111111, // 0
    SEG7_ONE   = 0b00000110, // 1
    SEG7_TWO   = 0b01011011, // 2
    SEG7_THREE = 0b01001111, // 3
    SEG7_FOUR  = 0b01100110, // 4
    SEG7_FIVE  = 0b01101101, // 5
    SEG7_SIX   = 0b01111101, // 6
    SEG7_SEVEN = 0b00000111, // 7
    SEG7_EIGHT = 0b01111111, // 8
    SEG7_NINE  = 0b01101111, // 9
    SEG7_A     = 0b01110111, // A
    SEG7_B     = 0b01111100, // b (lowercase for readability)
    SEG7_C     = 0b00111001, // C
    SEG7_D     = 0b01011110, // d (lowercase for readability)
    SEG7_E     = 0b01111001, // E
    SEG7_F     = 0b01110001, // F
    SEG7_OFF   = 0b00000000  // All segments off
};


static const seg7_pattern_t hex_map[] = {
    SEG7_ZERO, SEG7_ONE, SEG7_TWO, SEG7_THREE,
    SEG7_FOUR, SEG7_FIVE, SEG7_SIX, SEG7_SEVEN,
    SEG7_EIGHT, SEG7_NINE, SEG7_A, SEG7_B,
    SEG7_C, SEG7_D, SEG7_E, SEG7_F
};


seg7_pattern_t current_tens; // tens syftar på tiotals-siffran
seg7_pattern_t current_units; // units syftar på entals-siffran


/**
 * @brief Get segment pattern for a hex nibble (0-15)
 */
seg7_pattern_t get_segment_data(uint8_t nibble) {
    if (nibble > 0xF) {
        return SEG7_OFF;
    }
    return hex_map[nibble];
}


void extract_digits(uint8_t number, uint8_t *digit_tens, uint8_t *digit_units) {
    if (number > 99) {
        *digit_tens = 0;
        *digit_units = 0;
        return;
    }
    *digit_tens = number / 10;
    *digit_units = number % 10;
}

seg7_pattern_t bitsToRefresh(seg7_pattern_t current, seg7_pattern_t new) {
    return current & new;
}

seg7_pattern_t bitsToTurnOn(seg7_pattern_t current, seg7_pattern_t new) {
    return ~current & new;
}

seg7_pattern_t bitsToTurnOff(seg7_pattern_t current, seg7_pattern_t new) {
    return current & ~new;
}

void updateScreen(int temp){
    uint8_t digit_tens;
    uint8_t digit_units;
    extract_digits(temp,digit_tens,digit_units);
    seg7_pattern_t display_tens = get_segment_data(digit_tens);
    seg7_pattern_t display_units = get_segment_data(digit_units);

    seg7_pattern_t refresh_tens = bitsToRefresh(current_tens, display_tens);
    seg7_pattern_t turn_on_tens = bitsToTurnOn(current_tens, display_tens);
    seg7_pattern_t turn_off_tens = bitsToTurnOff(current_tens, display_tens);

    seg7_pattern_t refresh_units = bitsToRefresh(current_units, display_units);
    seg7_pattern_t turn_on_units = bitsToTurnOn(current_units, display_units);
    seg7_pattern_t turn_off_units = bitsToTurnOff(current_units, display_units);

}

