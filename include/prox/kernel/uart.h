/**
 * ProxOS Kernel UART
 *
 * Raw character output over the PL011 UART on the QEMU virt machine.
 *
 * Copyright 2026 Preston Brown
 */

#ifndef PROX_KERNEL_UART_H
#define PROX_KERNEL_UART_H

// Prepares the UART for output
void uart_init(void);

// Writes a single character, blocks while the transmit fifo is full
void uart_putc(char value);

#endif
