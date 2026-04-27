/* stm32411re.ld
 * Linker script for STM32F411RE (Nucleo-F411RE), bare-metal.
 *
 * Memory map (RM0383, §5):
 *   FLASH  0x0800 0000  512 KB  (rx)   – code + read-only data + .data LMA
 *   SRAM   0x2000 0000  128 KB  (xrw)  – .data VMA + .bss + stack
 *
 * Stack: grows downward from the top of SRAM (_estack = 0x2002 0000).
 * Reset_Handler copies .data from FLASH to SRAM, zeroes .bss, then
 * calls main().
 */

/* ── Entry point ─────────────────────────────────────────────────────────── */
ENTRY(Reset_Handler)

/* ── Memory regions ──────────────────────────────────────────────────────── */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* ── Top of stack ────────────────────────────────────────────────────────── */
/* Vector table entry [0]: initial SP value loaded by the CPU on reset.      */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);    /* = 0x20020000 */

/* ── Heap / stack size hints (used by nano-libs if linked) ───────────────── */
__max_heap_size  = 0x200;   /* 512 bytes  */
__max_stack_size = 0x400;   /* 1024 bytes */

/* ── Output sections ─────────────────────────────────────────────────────── */
SECTIONS
{
    /* ── 1. Interrupt vector table (must start at 0x08000000) ── */
    /* The startup file places the table in section ".isr_vector".           */
    /* KEEP() prevents the linker from discarding it (no direct reference).  */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* ── 2. Code + read-only data ── */
    .text :
    {
        . = ALIGN(4);
        *(.text)            /* code from all object files   */
        *(.text*)           /* inlined / LTO fragments      */
        *(.rodata)          /* string literals, const arrays */
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;         /* end of FLASH-resident content */
    } > FLASH

    /* ── 3. Initialised data (.data) ── */
    /* VMA (run address) = SRAM; LMA (load/store address) = FLASH.           */
    /* Reset_Handler copies [_sidata … _sidata+size) → [_sdata … _edata).   */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;         /* VMA start – destination for the copy loop    */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;         /* VMA end                                       */
    } > SRAM AT > FLASH

    /* LMA start of .data (source for the copy loop in Reset_Handler).       */
    /* Must be defined AFTER the section so LOADADDR() resolves correctly.   */
    _sidata = LOADADDR(.data);

    /* ── 4. Uninitialised data (.bss) ── */
    /* Reset_Handler zeroes [_sbss … _ebss) before calling main().           */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM
}
