/*
 * write code to allow blinking using arbitrary pins.
 * Implement:
 *  - gpio_set_output(pin) --- set GPIO <pin> as an output (vs input) pin.
 *  - gpio_set_on(pin) --- set the GPIO <pin> on.
 *  - gpio_set_off(pin) --- set the GPIO <pin> off.
 * Use the minimal number of loads and stores to GPIO memory.
 *
 * start.s defines a of helper functions (feel free to look at the assembly!  it's
 *  not tricky):
 *      uint32_t get32(volatile uint32_t *addr)
 *              --- return the 32-bit value held at <addr>.
 *
 *      void put32(volatile uint32_t *addr, uint32_t v)
 *              -- write the 32-bit quantity <v> to <addr>
 *
 * Check-off:
 *  1. get a single LED to blink.
 *  2. attach an LED to pin 19 and another to pin 20 and blink in opposite order (i.e.,
 *     one should be on, while the other is off).   Note, if they behave weirdly, look
 *     carefully at the wording for GPIO set.
 */
#include "rpi.h"

/*
 * These routines are given by us and are in start.s
 */
// writes the 32-bit value <v> to address <addr>:   *(unsigned *)addr = v;
void put32(volatile void* addr, unsigned v);
// returns the 32-bit value at <addr>:  return *(unsigned *)addr
unsigned get32(const volatile void* addr);
// does nothing.
void nop(void);

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
volatile unsigned* gpio_fsel0 = (void*)(GPIO_BASE + 0x00);
volatile unsigned* gpio_set0 = (void*)(GPIO_BASE + 0x1C);
volatile unsigned* gpio_clr0 = (void*)(GPIO_BASE + 0x28);
volatile unsigned* gpio_lev0 = (void*)(GPIO_BASE + 0x34);

// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output

// set <pin> to be an output pin.  note: fsel0, fsel1, fsel2 are contiguous in memory,
// so you can use array calculations!
void gpio_set_output(unsigned pin)
{
    if (pin > 53)
        return;
    unsigned raw_bit = pin * 3;
    unsigned offset = raw_bit / 30;
    unsigned shift = raw_bit % 30;
    volatile unsigned* addr = gpio_fsel0 + offset;
    unsigned val = get32(addr);
    // Clear the 3 bits
    val &= ~(0b111 << shift);
    // Set to 0b001
    val |= (0b001 << shift);
    put32(addr, val);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin)
{
    if (pin > 53)
        return;
    unsigned offset = pin / 32;
    unsigned shift = pin % 32;
    put32(gpio_set0 + offset, 1 << shift);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin)
{
    if (pin > 53)
        return;
    unsigned offset = pin / 32;
    unsigned shift = pin % 32;
    put32(gpio_clr0 + offset, 1 << shift);
}

// Part 2: implement gpio_set_input and gpio_read

// set <pin> to input.
void gpio_set_input(unsigned pin)
{
    if (pin > 53)
        return;
    unsigned raw_bit = pin * 3;
    unsigned offset = raw_bit / 30;
    unsigned shift = raw_bit % 30;
    volatile unsigned* addr = gpio_fsel0 + offset;
    unsigned val = get32(addr);
    // Clear the 3 bits
    val &= ~(0b111 << shift);
    // Set to 0b000
    val |= (0b000 << shift);
    put32(addr, val);
}

// return the value of <pin>
int gpio_read(unsigned pin)
{
    unsigned offset = pin / 32;
    unsigned shift = pin % 32;
    unsigned cur_val = get32(gpio_lev0 + offset);
    unsigned v = (cur_val & (1 << shift)) >> shift;
    return v;
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v)
{
    if (pin > 53)
        return;
    if (v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

void gpio_set_function(unsigned pin, gpio_func_t function)
{
    if (pin > 53)
        return;
    if (function > 7)
        return;
    unsigned raw_bit = pin * 3;
    unsigned offset = raw_bit / 30;
    unsigned shift = raw_bit % 30;
    volatile unsigned* addr = gpio_fsel0 + offset;
    unsigned val = get32(addr);
    // Clear the 3 bits
    val &= ~(0b111 << shift);
    // Set to function
    val |= (function << shift);
    put32(addr, val);
}
