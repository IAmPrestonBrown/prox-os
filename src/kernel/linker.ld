/**
 * ProxOS Kernel linker script
 *
 * The QEMU virt machine puts ram at 0x40000000. QEMU loads the kernel a
 * little above that so the boot info it writes below is left alone.
 */

ENTRY(_start)

SECTIONS
{
    . = 0x40080000;

    .text : {
        KEEP(*(.text.boot))
        *(.text*)
    }

    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }

    . = ALIGN(8);
    __bss_start = .;
    .bss : {
        *(.bss*)
        *(COMMON)
    }
    . = ALIGN(8);
    __bss_end = .;

    /* heap handed to kernel_heap_init, 1 mib */
    . = ALIGN(16);
    __heap_start = .;
    . = . + 0x100000;
    __heap_end = .;

    /* boot stack, 64 kib, grows down from __stack_top */
    . = ALIGN(16);
    . = . + 0x10000;
    __stack_top = .;
}
