#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <stdio.h>


typedef uint8_t seg7_pattern_t;

seg7_pattern_t get_segment_data(uint8_t nibble);

void extract_digits(uint8_t number, uint8_t *digit_tens, uint8_t *digit_units);

seg7_pattern_t bitsToRefresh(seg7_pattern_t current, seg7_pattern_t new);
seg7_pattern_t bitsToTurnOn(seg7_pattern_t current, seg7_pattern_t new);
seg7_pattern_t bitsToTurnOff(seg7_pattern_t current, seg7_pattern_t new);

void updateScreen(int temp);

void refreshPins(const uint8_t *hardware_pins, seg7_pattern_t refresh_pins);
void turnOffPins(const uint8_t *hardware_pins, seg7_pattern_t turn_off_pins);
void turnOnPins(const uint8_t *hardware_pins, seg7_pattern_t turn_on_pins);
void disconnectPins(const uint8_t *hardware_pins, seg7_pattern_t filter);

#endif /* SCREEN_H */