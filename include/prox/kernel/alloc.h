/**
 * ProxOS Kernel allocator
 *
 * Supplies the malloc and free that the VM declares in prox/vm/vm.h.
 * The heap bounds come from the linker script.
 *
 * Copyright 2026 Preston Brown
 */

#ifndef PROX_KERNEL_ALLOC_H
#define PROX_KERNEL_ALLOC_H

// Claims the linker provided heap region, call once before any allocation
void kernel_heap_init(void);

// Returns the number of bytes still available in the heap
unsigned long kernel_heap_remaining(void);

void* malloc(unsigned long bytes);
void free(void* allocation);

#endif
