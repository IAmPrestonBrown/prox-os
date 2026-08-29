/**
 * ProxOS Kernel boot
 *
 * Parks every core except core zero, sets the stack, clears the bss,
 * then hands control to kernel_main.
 *
 * Copyright 2026 Preston Brown
 */

.section .text.boot
.global _start

_start:
    // park every core except core zero
    mrs     x0, mpidr_el1
    and     x0, x0, #0xff
    cbnz    x0, park

    // point the stack pointer at the top of the boot stack
    ldr     x0, =__stack_top
    mov     sp, x0

    // clear the bss one 8 byte word at a time
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
clear_bss:
    cmp     x0, x1
    b.hs    enter_kernel
    str     xzr, [x0], #8
    b       clear_bss

enter_kernel:
    bl      kernel_main

park:
    wfe
    b       park
