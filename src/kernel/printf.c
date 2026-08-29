/**
 * ProxOS Kernel printf
 *
 * Copyright 2026 Preston Brown
 */

#include <prox/kernel/printf.h>

void prox_vprintf(const char* format, va_list args) {
    // TODO
    (void)format;
    (void)args;
}

void prox_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    prox_vprintf(format, args);
    va_end(args);
}
