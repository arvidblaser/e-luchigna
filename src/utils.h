#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

int sleep_mcu(int32_t ms);
