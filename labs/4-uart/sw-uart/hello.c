// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "sw-uart.h"

void notmain(void)
{
    // strictly speaking: don't need this.
    uart_init();

    gpio_set_output(20);
    gpio_set_input(21);

    // use pin 20 for tx, 21 for rx
    sw_uart_t u = sw_uart_init(20, 21, 115200);
    for (int i = 0; i < 10; i++)
        sw_uart_putk(&u, "hello world\n");

    unsigned counter = 0;
    while (counter < 10) {
        int v = sw_uart_getc(&u, 5000000);
        if (v >= 0) {
            sw_uart_putc(&u, v);
        }
        counter++;
    }
}
