#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

#define READ_ERR -1
#define WRITE_ERR -2

#define I2C_CAM_ADDRESS 0x21
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define CAM_PWM_PIN 2

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

bool ov7670_read_reg(uint8_t reg, uint8_t* value) {
    // Write the register address (keep control of the bus)
    int ret = i2c_write_blocking(i2c_default, I2C_CAM_ADDRESS, &reg, 1,
                                false);  // true = repeated START

    if (ret < 0) {
        printf("Failed to write register address 0x%02X\n", reg);
        return false;
    }

    // Read the register value
    ret = i2c_read_blocking(i2c_default, I2C_CAM_ADDRESS, value, 1,
                            false);  // false = STOP afterwards

    if (ret < 0) {
        printf("Failed to read register address 0x%02X\n", reg);
        return false;
    }

    return (ret == 1);
}

bool device_at_address(uint8_t addr) {
    // Perform a 1-byte dummy read from the probe address. If a slave
    // acknowledges this address, the function returns the number of bytes
    // transferred. If the address byte is ignored, the function returns
    // -1.
    uint8_t rxdata;
    int ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);
    return (ret >= 0);
}

bool init_i2c(uint8_t sda_pin, uint8_t scl_pin, uint32_t baudrate) {
    i2c_init(i2c_default, baudrate);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
    return true;
}

bool init_cam(uint pwm_pin) {
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);

    pwm_set_clkdiv(slice_num, 1.0f);   // no division
    pwm_set_wrap(slice_num, 4);        // period = wrap+1 = 5 cycles → 25MHz
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pwm_pin), 2); // ~50% duty (2 or 3 of 5)
    pwm_set_enabled(slice_num, true);
}

int read_cam_reg(uint8_t cam_addr, uint8_t reg, uint8_t* value) {    
    if (!i2c_write_blocking(i2c_default, I2C_CAM_ADDRESS, &reg, 1, false))
        return READ_ERR;
    if (!i2c_read_blocking(i2c_default, I2C_CAM_ADDRESS, value, 1, false))
        return WRITE_ERR;
}

int main() {
    stdio_init_all();
    init_i2c(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
    init_cam(CAM_PWM_PIN);
    
    sleep_ms(250); // Wait for the camera to power up and be ready for I2C communication
    
    if (device_at_address(I2C_CAM_ADDRESS))
        printf("Found I2C device at address 0x%02X\n", I2C_CAM_ADDRESS);
    else
        printf("Failed to detect device at I2C address 0x%02X\n", I2C_CAM_ADDRESS);
    
    uint8_t pid, ver;
    int res;
    
    res = read_cam_reg(I2C_CAM_ADDRESS, 0x0A, &pid);
    if (res == READ_ERR)
        printf("Failed to read PID register\n");
    else if (res == WRITE_ERR)
        printf("Failed to write PID register\n");
    else
        printf("PID: 0x%02X\n", pid);

    res = read_cam_reg(I2C_CAM_ADDRESS, 0x0B, &ver);
    if (res == READ_ERR)
        printf("Failed to read VER register\n");
    else if (res == WRITE_ERR)
        printf("Failed to write VER register\n");
    else
        printf("VER: 0x%02X\n", ver);

    while (1) {
        tight_loop_contents();
    }
 
    return 0;
}