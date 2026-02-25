#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

/**
 * @brief Extract tens and units digits from a number.
 *
 * @param number The input number (0-99).
 * @param digit_tens Pointer to store the tens digit.
 * @param digit_units Pointer to store the units digit.
 */
void extract_digits(uint8_t number, uint8_t *digit_tens, uint8_t *digit_units);


void updateScreen(int temp);

#endif /* SCREEN_H */