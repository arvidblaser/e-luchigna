#include <zephyr/ztest.h>
#include "screen.h"

// build test first time: west build -d build/tests -b native_sim/native/64 tests/screen
// next times: west build -d build/tests.
// run test: west build -d build/tests -t run

ZTEST(screen_tests, test_extract_digits_23) {
    uint8_t digit_tens, digit_units;
    extract_digits(23, &digit_tens, &digit_units);
    zassert_equal(digit_tens, 2, "Tens digit should be 2");
    zassert_equal(digit_units, 3, "Units digit should be 3");
}

ZTEST(screen_tests, test_bitsToRefresh_2_to_3) {
    seg7_pattern_t current = 0b01011011;
    seg7_pattern_t new =     0b01001111;
    seg7_pattern_t result = bitsToRefresh(current, new);
    zassert_equal(result, 0b01001011, "Should refresh these bits");
}

ZTEST(screen_tests, test_bitsToTurnOn_2_to_3) {
    seg7_pattern_t current = 0b01011011;
    seg7_pattern_t new =     0b01001111;
    seg7_pattern_t result = bitsToTurnOn(current, new);
    zassert_equal(result, 0b00000100, "Should turn on this bit");
}

ZTEST(screen_tests, test_bitsToTurnOff_2_to_3) {
    seg7_pattern_t current = 0b01011011;
    seg7_pattern_t new =     0b01001111;
    seg7_pattern_t result = bitsToTurnOff(current, new);
    zassert_equal(result, 0b00010000, "Should turn off this bit");
}

ZTEST_SUITE(screen_tests, NULL, NULL, NULL, NULL, NULL);