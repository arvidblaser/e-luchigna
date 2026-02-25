#include <zephyr/ztest.h>
#include "screen.h"

// build test first time: west build -d build/tests -b native_posix tests/screen
// next time: west build -d build/tests.
// run test: west build -t run

ZTEST(screen_tests, test_extract_digits_23) {
    uint8_t digit_tens, digit_units;
    extract_digits(23, &digit_tens, &digit_units);
    zassert_equal(digit_tens, 2, "Tens digit should be 2");
    zassert_equal(digit_units, 3, "Units digit should be 3");
}

ZTEST_SUITE(screen_tests, NULL, NULL, NULL, NULL, NULL);