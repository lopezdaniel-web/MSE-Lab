/**
 * @file    pwm.h
 * @brief   PWM output module – TIM1 Channel 1, pin PA8 (AF1).
 *
 * Hardware mapping (fixed):
 *   Timer   : TIM1, Channel 1  (APB2, advanced-control timer)
 *   Pin     : PA8  →  AF1 = TIM1_CH1
 *   Clock   : 16 MHz (HSI, APB2 prescaler = 1)
 *   PSC     : 15   →  tick = 1 µs
 *   ARR     : (1 000 000 / frequency_hz) − 1
 *
 * Duty-cycle range : 0–100 %
 * NOTE: TIM1 requires BDTR.MOE = 1 before any channel output appears.
 *       pwm_init() handles this automatically.
 */

#ifndef PWM_H
#define PWM_H

#include <stdint.h>

/**
 * @brief  Initialise TIM1 CH1 and PA8 for PWM output at the given frequency.
 *
 *  - Enables GPIOA and TIM1 peripheral clocks.
 *  - Configures PA8 as AF1 (TIM1_CH1) with high-speed output.
 *  - Calculates ARR from frequency (PSC = 15).
 *  - Configures TIM1 CH1 in PWM Mode 1 with preload.
 *  - Sets initial duty cycle to 0 %.
 *  - Enables CH1 output and the TIM1 main output (MOE).
 *  - Does NOT start the timer; call pwm_start() afterwards.
 *
 * @param  frequency_hz  Desired PWM frequency in Hz (e.g. 1000).
 *                       Valid range: 1 Hz … 500 kHz (at 16 MHz clock).
 */
void pwm_init(uint32_t frequency_hz);

/**
 * @brief  Set the PWM duty cycle.
 *
 *         CCR1 = period × duty_cycle_pct / 100.
 *         Update takes effect at the next timer update event (OC1PE set).
 *
 * @param  duty_cycle_pct  Percentage 0–100. Values > 100 are clamped to 100.
 */
void pwm_set_signal(uint8_t duty_cycle_pct);

/** @brief  Enable TIM1 counter – PWM signal begins toggling on PA8. */
void pwm_start(void);

/** @brief  Disable TIM1 counter – PA8 output freezes in its current state. */
void pwm_stop(void);

#endif /* PWM_H */
