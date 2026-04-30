/**
 * @file    timer.h
 * @brief   Millisecond blocking-delay module built on TIM2.
 *
 * Hardware configuration (fixed):
 *   Timer   : TIM2  (APB1, 32-bit general-purpose)
 *   Clock   : 16 MHz (HSI, APB1 prescaler = 1)
 *   PSC     : 15   →  tick period = (15+1)/16 MHz = 1 µs
 *   ARR     : 999  →  overflow every (999+1) × 1 µs = 1 ms
 *
 * Usage:
 *   timer_init();           // call once at startup
 *   timer_delay_ms(500);    // blocks for 500 ms
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/**
 * @brief  Initialise TIM2 as a 1 ms tick counter.
 *         Enables APB1 clock for TIM2, sets PSC/ARR, starts counter.
 *         Must be called before any timer_delay_ms() calls.
 */
void timer_init(void);

/**
 * @brief  Block execution for the specified number of milliseconds.
 *
 *         Uses polling on TIM2 UIF (no interrupts required).
 *         Each iteration resets the counter and waits for one 1 ms
 *         overflow, giving ~1 ms accuracy per step.
 *
 * @param  ms  Duration in milliseconds (0 returns immediately).
 */
void timer_delay_ms(uint32_t ms);

#endif /* TIMER_H */
