#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1

char str_buf[1024];

void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);

        if (uart_is_writable(UART_ID)) {            
            snprintf(str_buf, 1024, "Recieved byte %d from driver\n\r", ch);
            uart_puts(uart0, str_buf);
        }
    }
}

int main() {
    uart_init(UART_ID, 115200);

    // Tx to computer
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    // Rx from driver
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, false);

    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    uart_set_irq_enables(UART_ID, true, false);

    uart_puts(UART_ID, "\n\rUART START\n\r");

    while (true) {
        tight_loop_contents();
    }
}