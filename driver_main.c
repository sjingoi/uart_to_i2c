#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART0_TX_PIN 0
#define UART0_RX_PIN 1
#define UART1_TX_PIN 8
#define UART1_RX_PIN 9

char str_buf[1024];

void on_uart0_rx() {
    while (uart_is_readable(uart0)) {
        uint8_t ch = uart_getc(uart0);

        if (uart_is_writable(uart1)) {
            // Forward to converter pico
            uart_putc(uart1, ch);
            
            snprintf(str_buf, 1024, "Sent byte %d\n\r", ch);
            uart_puts(uart0, str_buf);
        }
    }
}

int main() {
    
    // Set up UART connection to computer
    uart_init(uart0, 115200);
    gpio_set_function(UART0_TX_PIN, UART_FUNCSEL_NUM(uart0, UART0_TX_PIN));
    gpio_set_function(UART0_RX_PIN, UART_FUNCSEL_NUM(uart0, UART0_RX_PIN));
    
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, false);
    
    irq_set_exclusive_handler(UART0_IRQ, on_uart0_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);
    
    // Set up UART connection to converter pico
    uart_init(uart1, 115200);
    gpio_set_function(UART1_TX_PIN, UART_FUNCSEL_NUM(uart1, UART1_TX_PIN));

    uart_set_hw_flow(uart1, false, false);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart1, false);

    uart_puts(uart0, "\n\rUART START\n\r");

    while (true) {
        tight_loop_contents();
    }
}