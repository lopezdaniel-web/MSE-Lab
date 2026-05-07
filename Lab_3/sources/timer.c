/**
 * @file    timer.c
 * @brief   Millisecond blocking-delay module using TIM2 (polling, no IRQ).
 *
 * Clock derivation (HSI, all prescalers = 1 after reset):
 *
 *   SYSCLK = 16 MHz (HSI)
 *   AHB    = 16 MHz (HPRE  = /1)
 *   APB1   = 16 MHz (PPRE1 = /1)  →  TIM2_CLK = 16 MHz
 *
 * Timer configuration:
 *   PSC = 15   →  tick = (PSC+1) / TIM2_CLK = 16 / 16 MHz = 1 µs
 *   ARR = 999  →  overflow = (ARR+1) × tick  = 1000 × 1 µs = 1 ms
 *
 * Delay strategy (timer_delay_ms):
 *   For each millisecond:
 *     1. Clear UIF (avoid false-positive from a previous overflow).
 *     2. Reset CNT to 0 (start fresh 1 ms window).
 *     3. Poll UIF until the counter reaches ARR and wraps → UIF = 1.
 */

#include "timer.h"
#include "tim_driver.h"
#include "stm32f411xe.h"

/* ====================================================================
 * Private Constants
 * ==================================================================== */

#define TIMER_PSC   (15U)   /**< Prescaler: divides 16 MHz → 1 MHz tick  */
#define TIMER_ARR   (999U)  /**< Auto-reload: 1000 ticks = 1 ms overflow */

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief  Configure TIM2 for 1 ms tick and start the counter.
 *
 * Call sequence:
 *  1. Enable TIM2 peripheral clock on APB1 bus.
 *  2. Call tim_init() to write PSC/ARR and clear UIF.
 *  3. Enable the counter with tim_enable() – runs continuously.
 */
void timer_init(void)
{
    /* Enable TIM2 clock on APB1 */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* PSC=15, ARR=999 → 1 ms overflow at 16 MHz */
    tim_init(TIM2, TIMER_PSC, TIMER_ARR);

    /* Start the free-running counter */
    tim_enable(TIM2);
}

/**
 * @brief  Block for @p ms milliseconds using TIM2 overflow polling.
 *
 * Each loop iteration corresponds to exactly one 1 ms counter period.
 * The counter is reset at the start of each iteration to guarantee a
 * full 1 ms window regardless of when the previous iteration ended.
 *
 * Accuracy note:
 *   - Each 1 ms step has ±1 tick (~1 µs) jitter from the reset latency.
 *   - For n steps the accumulated error is bounded to ±n µs.
 *   - This is sufficient for LED blink and similar non-critical timing.
 *
 * @param  ms  Number of milliseconds to wait. 0 returns immediately.
 */
void timer_delay_ms(uint32_t ms)
{
    uint32_t i;

    for (i = 0U; i < ms; i++)
    {
        /* Clear the flag BEFORE resetting CNT so any pending overflow
         * from the previous iteration does not trigger an early exit. */
        tim_clear_update_flag(TIM2);
        tim_reset_counter(TIM2);

        /* Busy-wait until the counter overflows (CNT reaches ARR+1). */
        while (tim_get_update_flag(TIM2) == 0U)
        {
            /* No other work here – this is a pure blocking delay. */
        }
    }
}
