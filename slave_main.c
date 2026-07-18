#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"
#include "hardware/pwm.h"

#define I2C_PORT i2c0
#define SLAVE_ADDRESS 0x44

#define I2C_SDA_GPIO_S 4
#define I2C_SCL_GPIO_S 5

void i2c_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    puts("Something happened...\n");
    switch (event) {

    case I2C_SLAVE_RECEIVE:
        // Master wrote one byte
        uint8_t b = i2c_read_byte_raw(i2c);
        // Process byte...
        break;

    case I2C_SLAVE_REQUEST:
        // Master is reading
        i2c_write_byte_raw(i2c, 0x55);
        break;

    case I2C_SLAVE_FINISH:
        // Transaction ended
        break;
    }
}

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 100000);

    gpio_set_function(I2C_SDA_GPIO_S, GPIO_FUNC_I2C); // SDA
    gpio_set_function(I2C_SCL_GPIO_S, GPIO_FUNC_I2C); // SCL

    i2c_slave_init(I2C_PORT, SLAVE_ADDRESS, i2c_handler);


    const uint PWM_PIN = 2;
    
    // 2. Set the GPIO function to PWM
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);

    pwm_set_clkdiv(slice_num, 1.0f);   // no division
    pwm_set_wrap(slice_num, 4);        // period = wrap+1 = 5 cycles → 25MHz
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PWM_PIN), 2); // ~50% duty (2 or 3 of 5)
    pwm_set_enabled(slice_num, true);

    puts("Slave ready, waiting for master1..\n");


    while (true)
        tight_loop_contents();
}