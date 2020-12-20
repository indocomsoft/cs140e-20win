// engler, cs140e: trivial example of how to log GET32.
//
// part 1: add get32, put32, PUT32.
//    - you'll have to modify the makefile --- search for GET32 and add there.
//    - simply have put32 call PUT32, get32 call GET32 so its easier to compare output.
//
// part 2: add a simple log for capturing
//
//  without capture it's unsafe to call during UART operations since infinitely
//  recursive and, also, will mess with the UART state.
//
//  record the calls in a log (just declare a statically sized array until we get malloc)
//  for later emit by trace_dump()
//
// part 3: keep adding more functions!
//      often it's useful to trace at a higher level, for example, doing
//      uart_get() vs all the put's/get's it does, since that is easier for
//      a person to descipher.  or, you can do both:
//          -  put a UART_PUTC_START in the log.
//          - record the put/gets as normal.
//          - put a UART_PUTC_STOP in the log.
//
// XXX: may have to use wdiff or similar to match the outputs up exactly.  or strip out
// spaces.
//

#include "trace.h"
#include "rpi.h"

// gross that these are not auto-consistent with GET32 in rpi.h
unsigned __wrap_GET32(unsigned addr);
unsigned __real_GET32(unsigned addr);

unsigned __wrap_get32(volatile void* addr);
unsigned __real_get32(volatile void* addr);

void __wrap_PUT32(unsigned addr, unsigned v);
void __real_PUT32(unsigned addr, unsigned v);

void __wrap_put32(volatile void* addr, unsigned v);
void __real_put32(volatile void* addr, unsigned v);

static int tracing_p = 0;
static int in_trace = 0;
static int capture_p = 0;

typedef enum {
    OP_GET32 = 0,
    OP_PUT32 = 1,
} op_t;

typedef struct {
    const volatile void* addr;
    unsigned val;
} mem_t;

typedef struct {
    op_t op;
    mem_t v;
} log_ent_t;

#define BUF_SIZE 4096
static int used_buf = 0;
static log_ent_t buf[BUF_SIZE];

void trace_start(int local_capture_p)
{
    capture_p = local_capture_p;
    demand(!tracing_p, "trace already running!");
    demand(!in_trace, "invalid in_trace");
    tracing_p = 1;
}

// the linker will change all calls to GET32 to call __wrap_GET32
unsigned __wrap_GET32(unsigned addr)
{
    // the linker will change the name of GET32 to __real_GET32,
    // which we can then call ourselves.
    unsigned v = __real_GET32(addr);

    // doing this print while the UART is busying printing a character
    // will lead to an inf loop since printk will do its own
    // puts/gets.  use <in_trace> to skip our own monitoring calls.
    if (!in_trace && tracing_p) {
        in_trace = 1;
        // match it up with your unix print so you can compare.
        // we have to add a 0x
        printk("\tTRACE:GET32(0x%x)=0x%x\n", addr, v);
        in_trace = 0;
    }
    return v;
}

unsigned __wrap_get32(volatile void* addr)
{
    // the linker will change the name of GET32 to __real_GET32,
    // which we can then call ourselves.
    unsigned v = __real_get32(addr);

    // doing this print while the UART is busying printing a character
    // will lead to an inf loop since printk will do its own
    // puts/gets.  use <in_trace> to skip our own monitoring calls.
    if (!in_trace && tracing_p) {
        in_trace = 1;
        // match it up with your unix print so you can compare.
        // we have to add a 0x
        printk("\tTRACE:GET32(0x%x)=0x%x\n", addr, v);
        in_trace = 0;
    }
    return v;
}

void __wrap_PUT32(unsigned addr, unsigned v)
{
    __real_PUT32(addr, v);

    if (!in_trace && tracing_p) {
        in_trace = 1;
        printk("\tTRACE:PUT32(0x%p)=0x%x\n", addr, v);
        in_trace = 0;
    }
}

void __wrap_put32(volatile void* addr, unsigned v)
{
    __real_put32(addr, v);

    if (!in_trace && tracing_p) {
        in_trace = 1;
        printk("\tTRACE:PUT32(0x%p)=0x%x\n", addr, v);
        in_trace = 0;
    }
}

void trace_stop(void)
{
    demand(tracing_p, "trace already stopped!\n");
    tracing_p = 0;
}

void trace_dump(int reset_p)
{
    for (int i = 0; i < used_buf; i++) {
        switch (buf[i].op) {
        case OP_GET32:
            printk("\tTRACE:GET32(0x%x)=0x%x\n", buf[i].v.addr, buf[i].v.val);
            break;
        case OP_PUT32:
            printk("\tTRACE:PUT32(0x%x)=0x%x\n", buf[i].v.addr, buf[i].v.val);
            break;
        }
    }
    used_buf = 0;
    if (reset_p)
        trace_stop();
}
