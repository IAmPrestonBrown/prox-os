/**
 * ProxOS Kernel UART
 *
 * Copyright 2026 Preston Brown
 */

#include <prox/kernel/uart.h>

// PL011 base on the QEMU virt machine
#define UART_BASE 0x09000000UL

void uart_init(void) {
    // TODO
}

void uart_putc(char value) {
    // TODO
    (void)value;
}
