// implement:
//  void uart_init(void)
//
//  int uart_can_getc(void);
//  int uart_getc(void);
//
//  int uart_can_putc(void);
//  void uart_putc(unsigned c);
//
// see that hello world works.
//
//
#include "rpi.h"

struct {
    unsigned AUX_MU_IO_REG; // 40
    unsigned AUX_MU_IER_REG; // 44
    unsigned AUX_MU_IIR_REG; // 48
    unsigned AUX_MU_LCR_REG; // 4C
    unsigned AUX_MU_MCR_REG; // 50
    unsigned AUX_MU_LSR_REG; // 54
    unsigned AUX_MU_MSR_REG; // 58
    unsigned AUX_MU_SCRATCH; // 5C
    unsigned AUX_MU_CNTL_REG; // 60
    unsigned AUX_MU_STAT_REG; // 64
    unsigned AUX_MU_BAUD_REG; // 68
}* aux_uart = (void*)0x20215040;
unsigned* AUX_IRQ = (void*)0x20215000;
unsigned* AUX_ENABLES = (void*)0x20215004;

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void)
{
    unsigned val;
    // Disable mini UART first
    val = get32(AUX_ENABLES) & 0b111;
    dev_barrier();
    val &= ~1;
    put32(AUX_ENABLES, val);
    dev_barrier();
    // Set up GPIO
    gpio_set_function(14, GPIO_FUNC_ALT5);
    gpio_set_function(15, GPIO_FUNC_ALT5);
    dev_barrier();
    // Enable mini UART
    val = get32(AUX_ENABLES) & 0b111;
    dev_barrier();
    val |= 1;
    put32(AUX_ENABLES, val);
    dev_barrier();
    // Disable tx and rx
    val = get32(&aux_uart->AUX_MU_CNTL_REG);
    dev_barrier();
    val &= ~(0b11);
    put32(&aux_uart->AUX_MU_CNTL_REG, val);
    dev_barrier();
    // Set up 8n1 and clear DLAB bit to 0
    val = get32(&aux_uart->AUX_MU_LCR_REG);
    dev_barrier();
    val |= 0b11;
    val &= ~(1 << 7);
    put32(&aux_uart->AUX_MU_LCR_REG, val);
    dev_barrier();
    // Disable interrupts
    val = get32(&aux_uart->AUX_MU_IER_REG);
    dev_barrier();
    val &= 0b00;
    put32(&aux_uart->AUX_MU_IER_REG, val);
    dev_barrier();
    // Set up 115200 baud
    val = 250000000 / (8 * 115200) - 1;
    put32(&aux_uart->AUX_MU_BAUD_REG, val);
    dev_barrier();
    // Enable tx and rx
    val = get32(&aux_uart->AUX_MU_CNTL_REG);
    dev_barrier();
    val |= 0b11;
    put32(&aux_uart->AUX_MU_CNTL_REG, val);
    dev_barrier();
}

// 1 = at least one byte on rx queue, 0 otherwise
static int uart_can_getc(void)
{
    return get32(&aux_uart->AUX_MU_STAT_REG) & 1;
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_getc(void)
{
    while (!uart_can_getc())
        ;
    return get32(&aux_uart->AUX_MU_IO_REG) & 0xff;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_putc(void)
{
    return (get32(&aux_uart->AUX_MU_STAT_REG) >> 1) & 1;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
void uart_putc(unsigned c)
{
    while (!uart_can_putc())
        ;
    put32(&aux_uart->AUX_MU_IO_REG, c & 0xff);
}

// simple wrapper routines useful later.

// a maybe-more intuitive name for clients.
int uart_has_data(void)
{
    return uart_can_getc();
}

int uart_getc_async(void)
{
    if (!uart_has_data())
        return -1;
    return uart_getc();
}
