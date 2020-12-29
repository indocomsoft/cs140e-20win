#include "sw-uart.h"
#include "rpi.h"

static inline void inline_delay_us(unsigned usec)
{
    unsigned rb = GET32(0x20003004);
    while (1) {
        unsigned ra = GET32(0x20003004);
        if ((ra - rb) >= usec) {
            break;
        }
    }
}

// helper: cleans up the code.
static inline void timed_write(int pin, int v, unsigned usec)
{
    gpio_write(pin, v);

    inline_delay_us(usec);
}

// do this first: used timed_write to cleanup.
//  recall: time to write each bit (0 or 1) is in <uart->usec_per_bit>
void sw_uart_putc(sw_uart_t* uart, unsigned char c)
{
    int tx = uart->tx;
    int usec_per_bit = uart->usec_per_bit;
    int v;
    // send start bit
    timed_write(tx, 0, usec_per_bit);
    // send data
    v = (c >> 0) & 1;
    timed_write(tx, v, usec_per_bit);
    v = (c >> 1) & 1;
    timed_write(tx, v, usec_per_bit);
    v = (c >> 2) & 1;
    timed_write(tx, v, usec_per_bit);
    v = (c >> 3) & 1;
    timed_write(tx, v, usec_per_bit);
    v = (c >> 4) & 1;
    timed_write(tx, v, usec_per_bit);
    v = (c >> 5) & 1;
    timed_write(tx, v, usec_per_bit);
    v = (c >> 6) & 1;
    timed_write(tx, v, usec_per_bit);
    v = (c >> 7) & 1;
    timed_write(tx, v, usec_per_bit);
    // send end bit
    timed_write(tx, 1, 2 * usec_per_bit);
}

static inline int timed_read(int pin, unsigned usec)
{
    int v = gpio_read(pin);
    inline_delay_us(usec);
    return v;
}

// do this second: you can type in pi-cat to send stuff.
//      EASY BUG: if you are reading input, but you do not get here in
//      time it will disappear.
int sw_uart_getc(sw_uart_t* uart, int timeout_usec)
{
    int rx = uart->rx;
    int usec_per_bit = uart->usec_per_bit;
    int c;
    unsigned rb = GET32(0x20003004);

    // Read until the start bit is seen
    while (1) {
        unsigned ra = GET32(0x20003004);
        if ((ra - rb) >= timeout_usec) {
            return -1;
        }
        int val = gpio_read(rx);
        if (val == 0) {
            break;
        }
    }
    gpio_read(rx);
    inline_delay_us(usec_per_bit);
    inline_delay_us(usec_per_bit / 2);
    // Read data
    c = 0;
    c |= timed_read(rx, usec_per_bit) << 0;
    c |= timed_read(rx, usec_per_bit) << 1;
    c |= timed_read(rx, usec_per_bit) << 2;
    c |= timed_read(rx, usec_per_bit) << 3;
    c |= timed_read(rx, usec_per_bit) << 4;
    c |= timed_read(rx, usec_per_bit) << 5;
    c |= timed_read(rx, usec_per_bit) << 6;
    c |= timed_read(rx, usec_per_bit) << 7;
    return c;
}

void sw_uart_putk(sw_uart_t* uart, const char* msg)
{
    for (; *msg; msg++)
        sw_uart_putc(uart, *msg);
}
