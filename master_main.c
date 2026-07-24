#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "camera_capture.pio.h"

#define READ_ERR -1
#define WRITE_ERR -2

#define I2C_CAM_ADDRESS 0x21
#define CAM_PWM_PIN 2
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define VSYNC_PIN 8
#define DATA_PIN 10
#define PCLK_PIN (DATA_PIN + 8)
#define HREF_PIN (DATA_PIN + 9)

/* Use a much smaller frame to reduce memory use and transfer volume. */
#define WIDTH 160
#define HEIGHT 120
#define BYTES_PER_PX 2
#define FRAME_BYTES (WIDTH * HEIGHT * BYTES_PER_PX)

uint8_t frame_buf[FRAME_BYTES];
uint cap_sm;
int cap_dma_chan;
volatile bool frame_ready = false;

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

void init_cam_pio(PIO pio) {
    uint offset = pio_add_program(pio, &camera_capture_program);
    cap_sm = pio_claim_unused_sm(pio, true);

    pio_sm_config c = camera_capture_program_get_default_config(offset);

    sm_config_set_in_pins(&c, DATA_PIN);
    for (int i = 0; i < 10; i++)
        pio_gpio_init(pio, DATA_PIN + i);
    pio_sm_set_consecutive_pindirs(pio, cap_sm, DATA_PIN, 10, false);

    sm_config_set_jmp_pin(&c, HREF_PIN);
    sm_config_set_in_shift(&c, false, true, 8);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv(&c, 1.0f);

    pio_sm_init(pio, cap_sm, offset, &c);
    pio_sm_set_enabled(pio, cap_sm, true);
}

void dma_irq_handler() {
    dma_hw->ints0 = 1u << cap_dma_chan; // clear the interrupt request
    frame_ready = true; // set the flag to indicate that a new frame is ready
}

volatile uint32_t vsync_count = 0;

void vsync_irq_handler(uint gpio, uint32_t events) {
    if (gpio != VSYNC_PIN) return;
    vsync_count++;

    pio_sm_set_enabled(pio0, cap_sm, false); // stop the PIO state machine
    pio_sm_clear_fifos(pio0, cap_sm); // clear the PIO FIFOs
    pio_sm_restart(pio0, cap_sm); // restart the PIO state machine

    dma_channel_abort(cap_dma_chan); // abort any ongoing DMA transfer
    dma_channel_set_write_addr(cap_dma_chan, frame_buf, false); // set the DMA write address to the frame buffer
    dma_channel_set_trans_count(cap_dma_chan, FRAME_BYTES, false); // set the DMA transfer count to the frame size
    dma_channel_start(cap_dma_chan); // start the DMA transfer

    pio_sm_set_enabled(pio0, cap_sm, true); // re-enable the PIO state machine
}

void init_dma(PIO pio) {
    cap_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config dc = dma_channel_get_default_config(cap_dma_chan);
    channel_config_set_transfer_data_size(&dc, DMA_SIZE_8);
    channel_config_set_read_increment(&dc, false);
    channel_config_set_write_increment(&dc, true);
    channel_config_set_dreq(&dc, pio_get_dreq(pio, cap_sm, false));

    dma_channel_configure(cap_dma_chan, &dc, frame_buf, &pio->rxf[cap_sm], FRAME_BYTES, false);

    dma_channel_set_irq0_enabled(cap_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

void init_vsync_irq() {
    gpio_init(VSYNC_PIN);
    gpio_set_dir(VSYNC_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(VSYNC_PIN, GPIO_IRQ_EDGE_RISE, true, vsync_irq_handler);
}

bool init_cam(uint pwm_pin) {
    gpio_init(PCLK_PIN);
    gpio_set_dir(PCLK_PIN, GPIO_IN);

    gpio_init(HREF_PIN);
    gpio_set_dir(HREF_PIN, GPIO_IN);

    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);

    pwm_set_clkdiv(slice_num, 1.0f);   // no division
    pwm_set_wrap(slice_num, 9);        
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pwm_pin), 5);
    pwm_set_enabled(slice_num, true);
}

// int read_cam_reg(uint8_t cam_addr, uint8_t reg, uint8_t* value) {    
//     if (!i2c_write_blocking(i2c_default, I2C_CAM_ADDRESS, &reg, 1, false))
//         return READ_ERR;
//     if (!i2c_read_blocking(i2c_default, I2C_CAM_ADDRESS, value, 1, false))
//         return WRITE_ERR;
//     return 0;
// }

int cam_write(uint8_t reg, uint8_t val) {
    uint8_t data[2] = {reg, val};
    if (!i2c_write_blocking(i2c_default, I2C_CAM_ADDRESS, data, 2, false))
        return WRITE_ERR;
    return 0;
}

void dump_image() {
    for (int i = 0; i < FRAME_BYTES; i += 100) {
        printf("%02X ", frame_buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
}

void init_cam_registers() {
    cam_write(0x12, 0x80); // reset
    sleep_ms(100);
    cam_write(0x12, 0x18); // RGB + QQVGA (much smaller frame)
    cam_write(0x40, 0xD0);
    cam_write(0x11, 0x01);
    cam_write(0x0C, 0x00);
    cam_write(0x3E, 0x00);
    cam_write(0x70, 0x3A);
    cam_write(0x71, 0x35);
    cam_write(0x72, 0x11);
    cam_write(0x73, 0xF1);
    cam_write(0xA2, 0x02);
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

    init_cam_registers();
    
    printf("Starting continuous capture...\n");

    init_cam_pio(pio0);
    init_dma(pio0);
    init_vsync_irq();
    dma_channel_start(cap_dma_chan); // start the DMA transfer

    printf("Waiting for frames...\n");

    while (true) {
        if (frame_ready) {
            frame_ready = false;
            printf("Frame captured!\n");
            dump_image();
            printf("Vsync count: %u\n", vsync_count);
        } else {
            printf("Frame not ready");
        }
        sleep_ms(200);
    }

    return 0;
}