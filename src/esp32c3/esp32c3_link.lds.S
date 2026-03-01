/* Linker script for esp32c3 (RISC-V RV32IMC) */

#include "autoconf.h"

ENTRY(esp32c3_main)

MEMORY
{
    /* Instruction RAM (256KB): code and read-only data */
    iram (RX)  : ORIGIN = 0x4037C000, LENGTH = 0x40000
    /* Data RAM (192KB): writable data, BSS, stack */
    dram (RW)  : ORIGIN = 0x3FCA0000, LENGTH = 0x30000
}

STACK_SIZE = 8192;

SECTIONS
{
    .text :
    {
        *(.text.esp32c3_main)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > iram

    .data :
    {
        _data_start = .;
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } > dram

    .bss :
    {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > dram

    _heap_start = _bss_end;
    _stack_end = ORIGIN(dram) + LENGTH(dram);
    _heap_end = _stack_end - STACK_SIZE;

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.note*)
    }
}
