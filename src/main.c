/*
* Gwen application with ADC + SHT4x + BLE + UART logging
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor/sht4x.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include "screen.h"
#include "button.h"

LOG_MODULE_REGISTER(my_logger, LOG_LEVEL_DBG);

/* Console for low-power suspend/resume */
#if CONFIG_SERIAL
static const struct device *cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
#endif

/* UART device for logging transmitted values */
static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/* ADC setup */
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified for ADC"
#endif
#define DT_SPEC_AND_COMMA(node_id, prop, idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};
static int16_t adc_buf;
static struct adc_sequence sequence = {
    .buffer = &adc_buf,
    .buffer_size = sizeof(adc_buf)
};
static uint16_t vcap_mv = 0;

/* SHT4X */
#if !DT_HAS_COMPAT_STATUS_OKAY(sensirion_sht4x)
#error "No sensirion,sht4x compatible node found in the device tree"
#endif
static const struct device *dev = DEVICE_DT_GET_ONE(sensirion_sht4x);

/* GPIOs */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
static const struct gpio_dt_spec temp_on = GPIO_DT_SPEC_GET(DT_NODELABEL(tmpon), gpios);

/* BLE */
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define BLE_ADVERTISE_TIME_MS 3100

static uint16_t mfg_data[5] = {0};
static const struct bt_data ad[] = { BT_DATA(BT_DATA_SVC_DATA16, mfg_data, sizeof(mfg_data)) };
static const struct bt_data sd[] = { BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN) };

/* Function prototypes */
static void bt_ready(int err);
static void ble_advertise(int16_t temp, uint16_t hum);
static int sleep_mcu(int32_t ms);
static int read_adc_mv(uint16_t *vcap_mv);
static void uart_transmit_log(const char *fmt, ...);

/* ---------------- Setup ---------------- */
void setup(void)
{
    /* GPIO init */
    if (!gpio_is_ready_dt(&led)) LOG_ERR("LED GPIO not ready");
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    if (!gpio_is_ready_dt(&temp_on)) LOG_ERR("Temp enable GPIO not ready");
    gpio_pin_configure_dt(&temp_on, GPIO_OUTPUT_ACTIVE);

    initScreenPins();
    setup_pushbutton();

    /* Give SHT4x time to power up */
    k_msleep(50);

    /* ADC channels setup */
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
        if (!device_is_ready(adc_channels[i].dev)) {
            LOG_ERR("ADC device not ready");
            return;
        }
        int err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0) {
            LOG_ERR("ADC channel setup failed (%d)", err);
            return;
        }
    }

    /* Bluetooth init */
    int err = bt_enable(bt_ready);
    if (err) LOG_ERR("Bluetooth init failed (%d)", err);
}

/* ---------------- Sleep ---------------- */
int sleep_mcu(int32_t ms)
{
#if CONFIG_SERIAL
    pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
#endif
    k_msleep(ms);
#if CONFIG_SERIAL
    pm_device_action_run(cons, PM_DEVICE_ACTION_RESUME);
#endif
    return 0;
}

/* ---------------- ADC ---------------- */
int read_adc_mv(uint16_t *vcap_mv)
{
    const struct adc_dt_spec *adc_spec = &adc_channels[0];
    sequence.channels = BIT(adc_spec->channel_id);
    sequence.resolution = adc_spec->resolution ? adc_spec->resolution : 12;
    sequence.oversampling = 0;

    if (!device_is_ready(adc_spec->dev)) return -ENODEV;
    int err = adc_read(adc_spec->dev, &sequence);
    if (err < 0) return err;

    int32_t val_mv = adc_buf;
    err = adc_raw_to_millivolts_dt(adc_spec, &val_mv);
    *vcap_mv = (err < 0) ? (uint16_t)adc_buf : (uint16_t)val_mv;

    pm_device_action_run(adc_spec->dev, PM_DEVICE_ACTION_SUSPEND);
    return 0;
}

/* ---------------- BLE ---------------- */
static void bt_ready(int err)
{
    if (err) {
        LOG_ERR("Bluetooth init failed (%d)", err);
        return;
    }
    LOG_INF("Bluetooth initialized");
}

void ble_advertise(int16_t temp, uint16_t hum)
{
    mfg_data[1] = temp;
    mfg_data[2] = hum;
    mfg_data[3] = vcap_mv;

    /* UART log of transmitted values */
    uart_transmit_log("BLE transmit: Temp=%d, Hum=%d, Vcap=%d mV\r\n", temp, hum, vcap_mv);

    int err = bt_le_adv_start(BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY,
                                              BT_GAP_ADV_SLOW_INT_MIN,
                                              BT_GAP_ADV_SLOW_INT_MAX,
                                              NULL),
                              ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising start failed (%d)", err);
        return;
    }

    sleep_mcu(BLE_ADVERTISE_TIME_MS);

    err = bt_le_adv_stop();
    if (err) LOG_ERR("Advertising stop failed (%d)", err);
}

/* ---------------- UART helper ---------------- */
#include <stdarg.h>
static void uart_transmit_log(const char *fmt, ...)
{
    if (!uart_dev || !device_is_ready(uart_dev)) return;

    char buf[128];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len > 0) {
        for (int i = 0; i < len; i++) {
            uart_poll_out(uart_dev, buf[i]);
        }
    }
}

/* ---------------- Main Loop ---------------- */
#define SLEEP_TIME_MS 10000
//#define ADC_MEASURE_INTERVAL_MS 300000   // 5 minutes
//#define ADC_MEASURE_INTERVAL_MS 60000 // 1 minute
#define ADC_MEASURE_INTERVAL_MS 0

void main(void)
{
    LOG_DBG("Jiva board started");
    setup();

    gpio_pin_set_dt(&led,1);
    gpio_pin_set_dt(&temp_on,1);
    sleep_mcu(100);
    LOG_DBG("LED 0");
    gpio_pin_set_dt(&led,0);

    int64_t last_adc_time = 0;
    struct sensor_value temp, hum;
    int16_t temp_as_int;
    uint16_t hum_as_uint;

    /* Voltage divider constants */
    const float Rtop = 1.0f;   // MΩ
    const float Rbot = 10.0f;  // MΩ
    const float divider_factor = (Rtop + Rbot) / Rbot; // 11/10 = 1.1

    while (1)
    {
        int64_t now = k_uptime_get();

        /* ---------- ADC every 5 minutes ---------- */
        if (now - last_adc_time >= ADC_MEASURE_INTERVAL_MS) {
            int rc = read_adc_mv(&vcap_mv);
            if (rc < 0) LOG_ERR("ADC read failed (%d)", rc);
            else {
                /* Convert ADC voltage back to actual supercap voltage */
                float vcap_actual = vcap_mv * divider_factor;
                vcap_mv = (uint16_t)(vcap_actual + 0.5f); // round to nearest mV
                uart_transmit_log("ADC read: Supercap=%d mV\r\n", vcap_mv);
            }
            last_adc_time = now;
        }

        /* ---------- Temp/Humidity ---------- */
        int rc = sensor_sample_fetch(dev);
        if (rc == 0) rc = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        if (rc == 0) rc = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum);
        if (rc != 0) LOG_ERR("Temp/hum sensor failed (%d)", rc);

        temp_as_int = (int16_t)(sensor_value_to_double(&temp) * 10);
        hum_as_uint = (uint16_t)(sensor_value_to_double(&hum) * 10);

        uart_transmit_log("Temp/Hum read: Temp=%d, Hum=%d\r\n", temp_as_int, hum_as_uint);

        switch (get_current_state())
        {
        case SHOW_TEMP:
            updateScreen((int)(sensor_value_to_double(&temp) + 0.5), false); // add 0.5 for rounding
            break;
        case SHOW_TEMP_UPSIDE_DOWN:
            updateScreen((int)(sensor_value_to_double(&temp) + 0.5), true); // add 0.5 for rounding
            break;
        case SHOW_HUM:
            updateScreen((int)(sensor_value_to_double(&hum) + 0.5), false); // add 0.5 for rounding
            break;
        case SHOW_HUM_UPSIDE_DOWN:
            updateScreen((int)(sensor_value_to_double(&hum) + 0.5), true); // add 0.5 for rounding
            break;
        default:
            updateScreen((int)(sensor_value_to_double(&temp) + 0.5), false); // add 0.5 for rounding
            break;
        }

        /* ---------- BLE advertise ---------- */
        ble_advertise(temp_as_int, hum_as_uint);

        /* ---------- Sleep ---------- */
        sleep_mcu(SLEEP_TIME_MS);
    }
}