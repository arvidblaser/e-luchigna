#include "screen.h"



#define REFRESH_TIME 100
#define TURN_ON_TIME 1500
#define TURN_OFF_TIME 1500


#if DT_NODE_EXISTS(DT_NODELABEL(display_11))
    // Only run this if the overlay is actually present
    #define HARDWARE_CONNECTED 1
#else
    // Fallback or do nothing for unit tests
#endif

#if HARDWARE_CONNECTED
LOG_MODULE_REGISTER(screen_logger, LOG_LEVEL_DBG);

//#define DISPLAY_11 DT_NODELABEL(display_11)
//static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
static const struct gpio_dt_spec display_15 = GPIO_DT_SPEC_GET(DT_NODELABEL(display_11), gpios);
static const struct gpio_dt_spec display_25 = GPIO_DT_SPEC_GET(DT_NODELABEL(display_11), gpios);


// tens syftar på tiotals-siffran
// units syftar på entals-siffran
static const struct gpio_dt_spec display_channels_units[] = { 
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_11), gpios),
        GPIO_DT_SPEC_GET(DT_NODELABEL(display_12), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_13), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_14), gpios),
		//GPIO_DT_SPEC_GET(DT_NODELABEL(display_15), gpios), // exclude com port
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_16), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_17), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_18), gpios),
	};

static const struct gpio_dt_spec display_channels_tens[] = { 
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_21), gpios),
        GPIO_DT_SPEC_GET(DT_NODELABEL(display_22), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_23), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_24), gpios),
		//GPIO_DT_SPEC_GET(DT_NODELABEL(display_25), gpios), // exclude com port
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_26), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_27), gpios),
		GPIO_DT_SPEC_GET(DT_NODELABEL(display_28), gpios),
	};
#endif


    // 0b0GFEDCBA
enum pin_segments {
    SEGMENT_C = 0b00000100, // C
    SEGMENT_B = 0b00000010, // B
    SEGMENT_A = 0b00000001, // A
    SEGMENT_D = 0b00001000, // D
    SEGMENT_F = 0b00010000, // F
    SEGMENT_G = 0b00100100, // G
    SEGMENT_E = 0b01000100, // E
};


// matches order of gpios
static const uint8_t segment_map[] = {
    SEGMENT_C, SEGMENT_B, SEGMENT_A, SEGMENT_D, SEGMENT_F, SEGMENT_G, SEGMENT_E
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

#if HARDWARE_CONNECTED
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


    turnOffPins(display_channels_tens, turn_off_tens);
    turnOffPins(display_channels_units, turn_off_units);

    turnOnPins(display_channels_tens, turn_on_tens);
    turnOnPins(display_channels_units, turn_on_units);
    k_msleep(TURN_ON_TIME);
    disconnectPins(display_channels_tens,turn_on_tens);
    disconnectPins(display_channels_units,turn_on_units);
     //Turning off takes some additional time than turning on. Working with timers/interrupts might be better solution
    k_msleep(TURN_OFF_TIME);
    disconnectPins(display_channels_tens,turn_off_tens);
    disconnectPins(display_channels_units,turn_off_units);
    

    refreshPins(display_channels_tens, refresh_tens);
    refreshPins(display_channels_units, refresh_units);
    k_msleep(REFRESH_TIME);
    disconnectPins(display_channels_tens,refresh_tens);
    disconnectPins(display_channels_units,refresh_units);

    current_units = display_units;
    current_tens = display_tens;

}

void initScreenPins(){
    if (!gpio_is_ready_dt(&display_15)) LOG_ERR("COM GPIO units not ready");
    gpio_pin_configure_dt(&display_15, GPIO_OUTPUT_INACTIVE);
    if (!gpio_is_ready_dt(&display_25)) LOG_ERR("COM GPIO tens not ready");
    gpio_pin_configure_dt(&display_25, GPIO_OUTPUT_INACTIVE);

    turnOffPins(display_channels_tens, 0b01111111);
    turnOffPins(display_channels_units, 0b01111111);

}

void refreshPins(const struct gpio_dt_spec *hardware_pins, seg7_pattern_t refresh_pins){
    for(int i=0; i<6;i++){
        if(segment_map[i] & refresh_pins){
			gpio_pin_configure_dt(&hardware_pins[i], GPIO_OUTPUT_ACTIVE);
        }
    }
}

void turnOffPins(const struct gpio_dt_spec *hardware_pins, seg7_pattern_t turn_off_pins){
    for(int i=0; i<6;i++){
        if(segment_map[i] & turn_off_pins){
			gpio_pin_configure_dt(&hardware_pins[i], GPIO_OUTPUT_INACTIVE);
        }
    }
}

void turnOnPins(const struct gpio_dt_spec *hardware_pins, seg7_pattern_t turn_on_pins){
    for(int i=0; i<6;i++){
        if(segment_map[i] & turn_on_pins){
			gpio_pin_configure_dt(&hardware_pins[i], GPIO_OUTPUT_ACTIVE);
        }
    }
}

void disconnectPins(const struct gpio_dt_spec *hardware_pins, seg7_pattern_t filter ){
    for(int i=0; i<6;i++){
         if(segment_map[i] & filter){
			gpio_pin_configure_dt(&hardware_pins[i], GPIO_DISCONNECTED);
         }
    }
}
#endif
