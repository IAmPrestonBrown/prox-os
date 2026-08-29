/**
 * ProxOS Kernel
 *
 * Entry point reached from boot.S once the stack and bss are ready.
 *
 * Copyright 2026 Preston Brown
 */

#ifndef PROX_KERNEL_KERNEL_H
#define PROX_KERNEL_KERNEL_H

// Kernel entry, boot.S calls this and parks the cpu if it returns
void kernel_main(void);

// Prints a message then parks the cpu, never returns
void kernel_panic(const char* message);

#endif
