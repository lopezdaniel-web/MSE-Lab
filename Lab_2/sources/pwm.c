/**
 * @file    pwm.c
 * @brief   PWM output module: TIM1 Channel 1 → PA8 (AF1, TIM1_CH1).
 *
 * ── Clock derivation (HSI, APB2 prescaler = 1 after reset) ──────────
 *   SYSCLK  = 16 MHz
 *   APB2    = 16 MHz  →  TIM1_CLK = 16 MHz
 *
 * ── Frequency / duty-cycle maths ────────────────────────────────────
 *   PSC = 15  →  tick = (15+1) / 16 MHz = 1 µs
 *   ARR = (1 000 000 / frequency_hz) − 1
 *   Period  = (ARR+1) × 1 µs = 1/frequency_hz seconds
 *   CCR1    = (ARR+1) × duty_pct / 100
 *
 *   Example – 1 kHz, 50 %:
 *     ARR  = 1 000 000/1 000 − 1 = 999
 *     CCR1 = 1000 × 50 / 100     = 500
 *
 * ── Initialisation sequence ─────────────────────────────────────────
 *   1. Enable GPIOA clock (AHB1) and TIM1 clock (APB2).
 *   2. Configure PA8 → AF1 (TIM1_CH1), push-pull, high-speed.
 *   3. tim_init(TIM1, PSC=15, ARR=calculated).
 *   4. tim_pwm_config(TIM1, CH1, MODE_1)  – sets OC1M, OC1PE, ARPE.
 *   5. tim_set_ccr(TIM1, CH1, 0)          – initial duty = 0 %.
 *   6. tim_channel_enable(TIM1, CH1)      – CCER.CC1E = 1.
 *   7. tim_enable_main_output(TIM1)       – BDTR.MOE  = 1  ← required!
 *   8. pwm_start() enables the counter.
 *
 * ── Why BDTR.MOE? ───────────────────────────────────────────────────
 *   TIM1 is an advanced-control timer.  Its channel outputs are gated by
 *   the "Main Output Enable" bit.  Without it, the PA8 pin stays at its
 *   GPIO idle level even if the counter is running and CC1E is set.
 */

#include "pwm.h"
#include "tim_driver.h"
#include "gpio_driver.h"
#include "stm32f411xe.h"

/* ====================================================================
 * Private Constants
 * ==================================================================== */

#define PWM_PSC         (15U)          /**< Prescaler → 1 µs tick at 16 MHz */
#define PWM_TIM_CLK_HZ  (16000000U)    /**< TIM1 source clock (APB2)         */
#define PWM_TICK_HZ     (PWM_TIM_CLK_HZ / (PWM_PSC + 1U))  /* 1 000 000 Hz  */

#define PWM_PA8_PIN     (8U)           /**< PA8 → TIM1_CH1                   */

/* ====================================================================
 * Private State
 * ==================================================================== */

/**
 * @brief  Stores (ARR + 1) = number of ticks per period.
 *         Used by pwm_set_signal() to compute CCR1 without needing
 *         to re-read the ARR register.
 */
static uint32_t g_pwm_period_ticks = 0U;

/**
 * @brief  Full initialisation of TIM1 CH1 on PA8 for PWM output.
 *
 * @param  frequency_hz  PWM carrier frequency in Hz (e.g. 1000 for 1 kHz).
 *                       Minimum: 1 Hz  (ARR = 999 999, requires 32-bit ARR –
 *                       TIM1 has only a 16-bit ARR, so practical min is ~16 Hz).
 *                       Maximum: 500 000 Hz (ARR = 1, 50 % → CCR1 = 1).
 */
void pwm_init(uint32_t frequency_hz)
{
    uint32_t arr;

    /* ── 1. Enable peripheral clocks ─────────────────────────────── */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   /* GPIOA */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;    /* TIM1  */

    /* ── 2. Configure PA8 as AF1 (TIM1_CH1) ─────────────────────── */
    gpio_init(GPIOA, PWM_PA8_PIN, GPIO_MODE_AF);
    gpio_set_output_type(GPIOA, PWM_PA8_PIN, GPIO_OTYPE_PP);
    gpio_set_speed(GPIOA, PWM_PA8_PIN, GPIO_SPEED_HIGH);
    gpio_set_pupd(GPIOA, PWM_PA8_PIN, GPIO_PUPD_NONE);
    gpio_set_af(GPIOA, PWM_PA8_PIN, GPIO_AF1);  /* AF1 = TIM1_CH1 */

    /* ── 3. Calculate ARR from requested frequency ───────────────── */
    /*
     *  tick_hz = TIM1_CLK / (PSC + 1) = 16 000 000 / 16 = 1 000 000 Hz
     *  ARR     = tick_hz / frequency_hz − 1
     */
    arr                  = (PWM_TICK_HZ / frequency_hz) - 1U;
    g_pwm_period_ticks   = arr + 1U;   /* Save for duty-cycle maths */

    /* ── 4. Initialise TIM1 base (PSC, ARR, reset CNT, force UEV) ─ */
    tim_init(TIM1, (uint16_t)PWM_PSC, arr);

    /* ── 5. Configure CH1 in PWM Mode 1 with preload ─────────────── */
    /*
     *  PWM Mode 1: output = HIGH while CNT < CCR1
     *                       LOW  while CNT ≥ CCR1
     *  OC1PE = 1 → CCR1 writes go to shadow; take effect at next UEV.
     *  ARPE  = 1 → ARR   also double-buffered (set inside tim_pwm_config).
     */
    tim_pwm_config(TIM1, TIM_CHANNEL_1, TIM_PWM_MODE_1);

    /* ── 6. Initial duty cycle = 0 % (CCR1 = 0) ─────────────────── */
    tim_set_ccr(TIM1, TIM_CHANNEL_1, 0U);

    /* ── 7. Enable CH1 output (CCER.CC1E) ───────────────────────── */
    tim_channel_enable(TIM1, TIM_CHANNEL_1);

    /* ── 8. Enable main output (BDTR.MOE) – mandatory for TIM1 ──── */
    tim_enable_main_output(TIM1);

    /* Timer counter is NOT started here; call pwm_start() next. */
}

/**
 * @brief  Set the PWM duty cycle (0–100 %).
 *
 *  Formula: CCR1 = period_ticks × duty_pct / 100
 *
 *  Edge cases:
 *    duty = 0  → CCR1 = 0  → output always LOW  (0 % duty)
 *    duty = 100 → CCR1 = ARR+1 → output always HIGH (100 % duty)
 *
 *  The new CCR1 value is written to the preload register and will
 *  become effective at the next counter update event (UEV).
 *
 * @param  duty_cycle_pct  0–100. Values above 100 are clamped to 100.
 */
void pwm_set_signal(uint8_t duty_cycle_pct)
{
    uint32_t ccr;

    if (duty_cycle_pct > 100U)
    {
        duty_cycle_pct = 100U;
    }

    ccr = (g_pwm_period_ticks * (uint32_t)duty_cycle_pct) / 100U;
    tim_set_ccr(TIM1, TIM_CHANNEL_1, ccr);
}

/**
 * @brief  Start the TIM1 counter; PWM signal begins appearing on PA8.
 */
void pwm_start(void)
{
    tim_enable(TIM1);
}

/**
 * @brief  Stop the TIM1 counter; PA8 output freezes at its current state.
 *         Call pwm_start() to resume without reconfiguring.
 */
void pwm_stop(void)
{
    tim_disable(TIM1);
}
