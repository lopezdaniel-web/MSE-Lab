/**
 ******************************************************************************
 * @file    STM32_startup.c
 * @brief   Minimal startup file for STM32F411RE (Cortex-M4).
 *
 * Responsibilities
 * ────────────────
 * 1. Declare the interrupt vector table in section ".isr_vector" so the
 *    linker places it at 0x08000000 (start of FLASH).
 * 2. Implement Reset_Handler:
 *      a. Call SystemInit()  – configure the internal clocks (from CMSIS).
 *      b. Copy .data from FLASH (LMA: _sidata) to SRAM (VMA: _sdata.._edata).
 *      c. Zero .bss (_sbss.._ebss).
 *      d. Call main().
 * 3. Provide a Default_Handler (infinite loop) for every unused IRQ via
 *    weak aliases so they can be overridden in application code.
 *
 * Linker symbols used
 * ───────────────────
 *  _estack  – initial stack pointer  (top of SRAM)
 *  _sidata  – LMA start of .data     (source in FLASH)
 *  _sdata   – VMA start of .data     (destination in SRAM)
 *  _edata   – VMA end   of .data
 *  _sbss    – start of .bss
 *  _ebss    – end   of .bss
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f4xx.h"          /* needs -DSTM32F411xE in CFLAGS             */
#include "system_stm32f4xx.h"   /* SystemInit() prototype                    */

/* ── Linker-exported symbols ─────────────────────────────────────────────── */
extern uint32_t _estack;    /* top of SRAM  (initial SP)                     */
extern uint32_t _sidata;    /* LMA start of .data in FLASH                   */
extern uint32_t _sdata;     /* VMA start of .data in SRAM                    */
extern uint32_t _edata;     /* VMA end   of .data in SRAM                    */
extern uint32_t _sbss;      /* start of .bss                                 */
extern uint32_t _ebss;      /* end   of .bss                                 */

/* ── Forward declarations ────────────────────────────────────────────────── */
void Reset_Handler   (void);
int  main            (void);
void Default_Handler (void);

/* ── Weak aliases – override any of these in your application code ───────── */
void NMI_Handler       (void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler (void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler (void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler  (void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler       (void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler  (void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler    (void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler   (void) __attribute__((weak, alias("Default_Handler")));

/* ── Interrupt vector table ──────────────────────────────────────────────── */
/* Placed in ".isr_vector"; the linker script maps this section first in
 * FLASH so entry [0] (initial SP) lands at 0x08000000, entry [1]
 * (Reset_Handler) at 0x08000004, etc.                                        */
uint32_t vector_tbl[] __attribute__((section(".isr_vector"))) =
{
    /* Cortex-M4 system exceptions */
    (uint32_t)&_estack,             /* [0]  Initial Stack Pointer             */
    (uint32_t)&Reset_Handler,       /* [1]  Reset                             */
    (uint32_t)&NMI_Handler,         /* [2]  NMI                               */
    (uint32_t)&HardFault_Handler,   /* [3]  HardFault                         */
    (uint32_t)&MemManage_Handler,   /* [4]  MemManage                         */
    (uint32_t)&BusFault_Handler,    /* [5]  BusFault                          */
    (uint32_t)&UsageFault_Handler,  /* [6]  UsageFault                        */
    0,                              /* [7]  Reserved                          */
    0,                              /* [8]  Reserved                          */
    0,                              /* [9]  Reserved                          */
    0,                              /* [10] Reserved                          */
    (uint32_t)&SVC_Handler,         /* [11] SVCall                            */
    (uint32_t)&DebugMon_Handler,    /* [12] DebugMonitor                      */
    0,                              /* [13] Reserved                          */
    (uint32_t)&PendSV_Handler,      /* [14] PendSV                            */
    (uint32_t)&SysTick_Handler,     /* [15] SysTick                           */
    /* STM32F411xE peripheral IRQs would follow here (all default for now)   */
};

/* ── Default handler ─────────────────────────────────────────────────────── */
/* Any unhandled exception/IRQ lands here and spins forever.
 * Attach a debugger and inspect LR / stack to identify the fault.           */
void Default_Handler(void)
{
    while (1);
}

/* ── Reset handler ───────────────────────────────────────────────────────── */
void Reset_Handler(void)
{
    /* ── a. System clock initialisation (HSI 16 MHz by default) ── */
    SystemInit();

    /* ── b. Copy initialised data from FLASH (LMA) to SRAM (VMA) ── */
    /* _sidata is the LMA start defined by LOADADDR(.data) in the linker    */
    /* script.  Without it Reset_Handler would read uninitialised memory.   */
    uint32_t *src = (uint32_t *)&_sidata;
    uint32_t *dst = (uint32_t *)&_sdata;
    uint32_t  len = ((uint32_t)&_edata - (uint32_t)&_sdata) / 4U;

    for (uint32_t i = 0U; i < len; i++)
    {
        *dst++ = *src++;
    }

    /* ── c. Zero-fill .bss ── */
    dst = (uint32_t *)&_sbss;
    len = ((uint32_t)&_ebss - (uint32_t)&_sbss) / 4U;

    for (uint32_t i = 0U; i < len; i++)
    {
        *dst++ = 0U;
    }

    /* ── d. Branch to application ── */
    main();

    /* Should never reach here; spin as a safety net. */
    while (1);
}
