/**
 * ProxOS Kernel
 *
 * Copyright 2026 Preston Brown
 */

#include <prox/kernel/alloc.h>
#include <prox/kernel/kernel.h>
#include <prox/kernel/printf.h>
#include <prox/kernel/uart.h>

void kernel_panic(const char* message) {
    // TODO report the message before parking
    (void)message;

    for(;;) {
        __asm__ volatile("wfe");
    }
}

void kernel_main(void) {
    uart_init();
    kernel_heap_init();

    // TODO
}
