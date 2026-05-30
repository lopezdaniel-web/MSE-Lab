/**
 * @file    timer.c
 * @brief   Millisecond blocking-delay using TIM5 (polling, no IRQ).
 *
 * TIM5 is on APB1 (16 MHz).
 * Strategy: configure TIM5 fresh for each requested delay (exactly
 * like the working reference implementation) — set PSC for 1 ms
 * resolution, ARR = delay_ms - 1, enable, poll UIF, disable.
 *
 * This avoids any accumulated counter drift between calls.
 */

#include "timer.h"
#include "tim_driver.h"
#include "stm32f411xe.h"

/* PSC for 1 ms tick: (16 000 000 / 1000) - 1 = 15999 */
#define TIMER_PSC   (15999U)

void timer_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;   /* Enable TIM5 APB1 clock */
}

/**
 * @brief  Block for ms milliseconds.
 *
 * Each call configures TIM5 with ARR = ms - 1 (one overflow = ms ticks
 * at 1 ms/tick), starts it, waits for UIF, then stops it.
 * Mirrors the verified tim_setTimerMs / tim_waitTimer pattern.
 */
void timer_delay_ms(uint32_t ms)
{
    if (ms == 0U) { return; }

    uint32_t arr = ms - 1U;
    if (arr > 65535U) { arr = 65535U; }   /* TIM5 is 32-bit but clamp for safety */

    tim_init(TIM5, (uint16_t)TIMER_PSC, arr);
    tim_enable(TIM5);

    while (tim_get_update_flag(TIM5) == 0U)
    {
        __NOP();
    }

    tim_disable(TIM5);
}