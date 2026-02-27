#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <stdio.h>

enum state {
  SHOW_TEMP = 0,
  SHOW_TEMP_UPSIDE_DOWN = 1,
  SHOW_HUM = 2,
  SHOW_HUM_UPSIDE_DOWN = 3,
  STATE_COUNT // Always keep this last
};

/* Public functions*/
enum state get_current_state();
void setup_pushbutton();

/* Internal functions */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);