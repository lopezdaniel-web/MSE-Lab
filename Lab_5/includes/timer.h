/**
 * @file    timer.h
 * @brief   Millisecond blocking-delay module built on TIM5.
 *
 * TIM5 is reserved exclusively for delays so it never
 * conflicts with TIM2 (used for PWM).
 *
 * PSC = (16000000 / 1000) - 1 = 15999  →  1 ms tick
 * ARR = delay_ms - 1                   →  overflow after delay_ms ms
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(void);
void timer_delay_ms(uint32_t ms);

#endif /* TIMER_H */