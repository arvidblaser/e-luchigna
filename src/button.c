
#include "button.h"


static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);
static struct gpio_callback button_cb_data;

static enum state current_state = SHOW_TEMP;  


enum state get_current_state(){
    return current_state;
}


void setup_pushbutton(){
    int ret;

    /* 1. Check if hardware is ready */
    if (!gpio_is_ready_dt(&button)) {
        return 0;
    }

    /* 2. Configure the GPIO pin */
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    
    /* 3. Initialize the callback structure with your function */
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    
    /* 4. Add the callback to the GPIO driver */
    gpio_add_callback(button.port, &button_cb_data);

    /* 5. Configure the interrupt type (Edge-to-Active) */
    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    current_state = (current_state + 1) % STATE_COUNT;
}