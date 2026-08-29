/**
 * ProxOS Kernel printf
 *
 * Supplies the prox_printf that the VM declares in prox/vm/vm.h.
 *
 * Copyright 2026 Preston Brown
 */

#ifndef PROX_KERNEL_PRINTF_H
#define PROX_KERNEL_PRINTF_H

#include <stdarg.h>

// Formats and writes to the UART, same call shape as printf
void prox_printf(const char* format, ...);

// Same as prox_printf but takes an already started argument list
void prox_vprintf(const char* format, va_list args);

#endif
