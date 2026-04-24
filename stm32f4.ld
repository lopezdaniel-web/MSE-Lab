/* Defining Firmware's entry point */
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Defining Top of the Stack */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    .text :
    {
        *(.isr_vector_tbl)
        *(.text)
        *(.rodata)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        . = ALIGN(4);
        _edata = .;
    } >SRAM AT>FLASH

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        . = ALIGN(4);
        _ebss = .;
    } >SRAM
}