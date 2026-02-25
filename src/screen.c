#include "screen.h"



#define REFRESH_TIME 100
#define TURN_ON_TIME 1500
#define TURN_OFF_TIME 1500


//#define DISPLAY_11 DT_NODELABEL(display_11)
//static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);


//static const struct gpio_dt_spec display_channels[] = { 
//		GPIO_DT_SPEC_GET(DT_NODELABEL(display_11), gpios),
//	};
// tens syftar på tiotals-siffran
// units syftar på entals-siffran

    // 0b0GFEDCBA
enum pin_segment_tens {
    DISPLAY_11_T = 0b00000100, // C
    DISPLAY_12_T = 0b00000010, // B
    DISPLAY_13_T = 0b00000001, // A
    DISPLAY_14_T = 0b00001000, // D
    // DISPLAY_15_T COM not a segment
    DISPLAY_16_T = 0b00010000, // F
    DISPLAY_17_T = 0b00100100, // G
    DISPLAY_18_T = 0b01000100, // E
};

enum pin_segment_units {
    DISPLAY_21_T = 0b00000100, // C
    DISPLAY_22_T = 0b00000010, // B
    DISPLAY_23_T = 0b00000001, // A
    DISPLAY_24_T = 0b00001000, // D
    // DISPLAY_25_T COM not a segment
    DISPLAY_26_T = 0b00010000, // F
    DISPLAY_27_T = 0b00100100, // G
    DISPLAY_28_T = 0b01000100, // E
};

static const uint8_t dummy_hardware_tens[] = {
    0,1,2,3,4,5,6};

static const uint8_t dummy_hardware_units[] = {
    0,1,2,3,4,5,6};

//consider chaning to abc...
static const uint8_t pin_map_tens[] = {
    DISPLAY_21_T, DISPLAY_22_T, DISPLAY_23_T, DISPLAY_24_T, DISPLAY_26_T, DISPLAY_27_T, DISPLAY_28_T
};


enum digit_segments {
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


seg7_pattern_t current_tens; 
seg7_pattern_t current_units; 


void k_msleep(int sleeptime) {
    //dummy function before zpehyr is setup properly
}

/**
 * @brief Get segment pattern for a hex nibble (0-15)
 */
seg7_pattern_t get_segment_data(uint8_t nibble) {
    if (nibble > 0xF) {
        return SEG7_OFF;
    }
    return hex_map[nibble];
}


/**
 * @brief Extract tens and units digits from a number.
 *
 * @param number The input number (0-99).
 * @param digit_tens Pointer to store the tens digit.
 * @param digit_units Pointer to store the units digit.
 */
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
    extract_digits(temp, &digit_tens, &digit_units);
    seg7_pattern_t display_tens = get_segment_data(digit_tens);
    seg7_pattern_t display_units = get_segment_data(digit_units);

    seg7_pattern_t refresh_tens = bitsToRefresh(current_tens, display_tens);
    seg7_pattern_t turn_on_tens = bitsToTurnOn(current_tens, display_tens);
    seg7_pattern_t turn_off_tens = bitsToTurnOff(current_tens, display_tens);

    seg7_pattern_t refresh_units = bitsToRefresh(current_units, display_units);
    seg7_pattern_t turn_on_units = bitsToTurnOn(current_units, display_units);
    seg7_pattern_t turn_off_units = bitsToTurnOff(current_units, display_units);


    turnOffPins(dummy_hardware_tens, turn_off_tens);
    turnOffPins(dummy_hardware_units, turn_off_units);

    turnOnPins(dummy_hardware_tens, turn_on_tens);
    turnOnPins(dummy_hardware_units, turn_on_units);
    k_msleep(TURN_ON_TIME);
    disconnectPins(dummy_hardware_tens,turn_on_tens);
    disconnectPins(dummy_hardware_units,turn_on_units);
     //Turning off takes some additional time than turning on. Working withe timers /interrupts might be better soloution
    k_msleep(TURN_OFF_TIME);
    disconnectPins(dummy_hardware_tens,turn_off_tens);
    disconnectPins(dummy_hardware_units,turn_off_units);
    

    refreshPins(dummy_hardware_tens, refresh_tens);
    refreshPins(dummy_hardware_units, refresh_units);
    k_msleep(REFRESH_TIME);
    disconnectPins(dummy_hardware_tens,refresh_tens);
    disconnectPins(dummy_hardware_units,refresh_units);

}

void refreshPins(const uint8_t *hardware_pins, seg7_pattern_t refresh_pins){
    for(int i=0; i<7;i++){
        if(pin_map_tens[i] & refresh_pins){
            // turnOn hardware_pins[i];
        }
    }
}

void turnOffPins(const uint8_t *hardware_pins, seg7_pattern_t turn_off_pins){
    for(int i=0; i<7;i++){
        if(pin_map_tens[i] & turn_off_pins){
            // turnOff hardware_pins[i];
        }
    }
}

void turnOnPins(const uint8_t *hardware_pins, seg7_pattern_t turn_on_pins){
    for(int i=0; i<7;i++){
        if(pin_map_tens[i] & turn_on_pins){
            // turnON hardware_pins[i];
        }
    }
}

void disconnectPins(const uint8_t *hardware_pins, seg7_pattern_t filter ){
    for(int i=0; i<7;i++){
         if(pin_map_tens[i] & filter){
            //disconnectPins
         }
    }
}

