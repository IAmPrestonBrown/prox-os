/**
 * ProxOS Kernel allocator
 *
 * Copyright 2026 Preston Brown
 */

#include <prox/kernel/alloc.h>

// heap bounds, filled in by the linker script
extern char __heap_start[];
extern char __heap_end[];

void kernel_heap_init(void) {
    // TODO
}

unsigned long kernel_heap_remaining(void) {
    // TODO
    return 0;
}

void* malloc(unsigned long bytes) {
    // TODO
    (void)bytes;
    return 0;
}

void free(void* allocation) {
    // TODO
    (void)allocation;
}
