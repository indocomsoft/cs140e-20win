/*
 * Simple tracing memory implementation.  Used for cross-checking.
 *
 * you will implement a simple fake memory that maps addresses to values.
 * e.g., an array, linked list, hash table of (<addr>, <val>) tuples such
 * that after you insert some (<addr>, <val>), you can lookup <val> using
 * <addr> as a key.
 *
 * simplistic assumptions:
 *  - all loads/stores are 32 bits.
 *  - read returns value of last write, or random() if none.
 *  - load and store prints the addr,val
 *
 * HINT: I woul suggest a fixed size array, that you do a simple linear scan
 * to insert, and lookup.
 *
 * mildly formal-ish rules for fake memory:
 *  1. no duplicate entries.
 *       if:
 *           (<addr>, <val0>) \in memory
 *           (<addr>, <val1>) \in memory
 *      then <val0> == <val1>
 *  2. addresses are persistent: if you ever write <addr>,<val> to memory,
 *     then <addr> stays there forever.
 *  3. values are persistent until the next write: if you ever write
 *     (<addr>,<val> ) to memory, then until the next write,
 *          if (<addr>, <v>) \in memory, then <v> == <val>.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "demand.h"
#include "pi-test.h"
#include "rpi.h"

/****************************************************************************
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 */

typedef struct {
    const volatile void* addr;
    unsigned val;
} mem_t;

typedef struct {
    int op;
    mem_t v;
} log_ent_t;

static mem_t mk_mem(const volatile void* addr, unsigned val)
{
    return (mem_t) { .addr = addr, .val = val };
}

// don't change routine so we can compare to everyone.
static void print_write(mem_t* m)
{
    printf("\tTRACE:PUT32(%p)=0x%x\n", m->addr, m->val);
}
// don't change routine so we can compare to everyone.
static void print_read(mem_t* m)
{
    printf("\tTRACE:GET32(%p)=0x%x\n", m->addr, m->val);
}

#define N 1024

size_t used = 0;
mem_t memory[N];

unsigned get32(const volatile void* addr)
{
    for (int i = 0; i < used; i++) {
        if (memory[i].addr == addr) {
            print_read(&memory[i]);
            return memory[i].val;
        }
    }
    demand(used < N, "N is not large enough");
    unsigned val = fake_random();
    memory[used] = mk_mem(addr, val);
    print_read(&memory[used]);
    used++;
    return val;
}

void put32(volatile void* addr, unsigned val)
{
    for (int i = 0; i < used; i++) {
        if (memory[i].addr == addr) {
            memory[i].val = val;
            print_write(&memory[i]);
            return;
        }
    }
    demand(used < N, "N is not large enough");
    memory[used] = mk_mem(addr, val);
    print_write(&memory[used]);
    used++;
}
