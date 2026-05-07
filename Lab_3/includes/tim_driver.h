/**
 * @file    tim_driver.h
 * @brief   Low-level TIM driver for STM32F4xx (CMSIS / bare-metal).
 *
 * Supports:
 *  - Timer initialisation (PSC, ARR, CNT, shadow register update)
 *  - Counter start / stop / reset / read
 *  - Update Interrupt Flag (UIF) poll and clear
 *  - PWM output mode configuration on channels 1–4
 *    (CCMRx OC mode, OCxPE preload, ARPE)
 *  - Capture/compare channel enable / disable (CCER CCxE)
 *  - Advanced-timer main output enable (BDTR MOE) for TIM1 / TIM8
 *
 * Clock assumptions (HSI 16 MHz, APBx prescaler = 1):
 *   TIM2_CLK = 16 MHz  (APB1)
 *   TIM1_CLK = 16 MHz  (APB2)
 *
 * Peripheral clock is NOT enabled by this driver; enable it in the
 * higher-level module (timer.c, pwm.c) before calling tim_init().
 */

#ifndef TIM_DRIVER_H
#define TIM_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>   /* for NULL */

/* ====================================================================
 * Constants
 * ==================================================================== */

/** @brief HSI-based APB1 timer clock (TIM2–TIM5, APB1 pre=1). */
#define TIM_APB1_CLOCK_HZ   (16000000U)

/** @brief HSI-based APB2 timer clock (TIM1, TIM9–TIM11, APB2 pre=1). */
#define TIM_APB2_CLOCK_HZ   (16000000U)

/* ====================================================================
 * Enumerations
 * ==================================================================== */

/** @brief Driver return status. */
typedef enum
{
    TIM_OK    = 0U, /**< Operation successful */
    TIM_ERROR = 1U  /**< Invalid argument or NULL pointer */
} TIM_Status_t;

/**
 * @brief Capture/compare channel selection.
 *        Numeric value maps directly to channel number (1–4).
 */
typedef enum
{
    TIM_CHANNEL_1 = 1U,
    TIM_CHANNEL_2 = 2U,
    TIM_CHANNEL_3 = 3U,
    TIM_CHANNEL_4 = 4U
} TIM_Channel_t;

/**
 * @brief PWM generation mode (value written to OC1M / OC2M field).
 *
 * Mode 1 (110b = 6): output HIGH while CNT < CCRx, LOW otherwise.
 * Mode 2 (111b = 7): output LOW  while CNT < CCRx, HIGH otherwise.
 */
typedef enum
{
    TIM_PWM_MODE_1 = 6U, /**< Active-high PWM (standard) */
    TIM_PWM_MODE_2 = 7U  /**< Active-low  PWM (inverted) */
} TIM_PWM_Mode_t;

/* ====================================================================
 * Public API – Core Timer Control
 * ==================================================================== */

/**
 * @brief  Initialise a timer with prescaler and auto-reload value.
 *
 * Timer tick   = (PSC + 1) / TIMx_CLK
 * Overflow per = (ARR + 1) × tick
 *
 * Writes PSC and ARR to shadow registers via EGR.UG, then clears UIF.
 * Timer is LEFT DISABLED after this call; call tim_enable() to start.
 *
 * @param  tim  TIM peripheral pointer (TIM1, TIM2, …).
 * @param  psc  Prescaler value (0 = divide by 1).
 * @param  arr  Auto-reload value (counter wraps at this value).
 * @return TIM_OK or TIM_ERROR.
 */
TIM_Status_t tim_init(TIM_TypeDef *tim, uint16_t psc, uint32_t arr);

/** @brief Enable the timer counter (set CR1.CEN). */
void tim_enable(TIM_TypeDef *tim);

/** @brief Disable the timer counter (clear CR1.CEN). */
void tim_disable(TIM_TypeDef *tim);

/** @brief Write 0 to the counter register. */
void tim_reset_counter(TIM_TypeDef *tim);

/** @brief Return the current 16- or 32-bit counter value. */
uint32_t tim_get_counter(TIM_TypeDef *tim);

/* ====================================================================
 * Public API – Status Flags
 * ==================================================================== */

/**
 * @brief  Read the Update Interrupt Flag (UIF) from SR.
 * @return 1 when the counter has overflowed since the last clear, else 0.
 */
uint8_t tim_get_update_flag(TIM_TypeDef *tim);

/** @brief Clear the UIF flag (write 0 to SR.UIF). */
void tim_clear_update_flag(TIM_TypeDef *tim);

/* ====================================================================
 * Public API – PWM Configuration
 * ==================================================================== */

/**
 * @brief  Configure a capture/compare channel for PWM output.
 *
 * Steps performed for the requested channel:
 *  1. Clear CCxS bits  → select output-compare mode (not input capture).
 *  2. Set OC1M/OC2M/… field to the requested PWM mode value (6 or 7).
 *  3. Set OCxPE         → enable CCRx preload (updates on UEV only).
 *  4. Set CR1.ARPE      → enable ARR preload.
 *
 * @param  tim      TIM peripheral pointer.
 * @param  channel  TIM_CHANNEL_1 … TIM_CHANNEL_4.
 * @param  mode     TIM_PWM_MODE_1 or TIM_PWM_MODE_2.
 * @return TIM_OK or TIM_ERROR.
 */
TIM_Status_t tim_pwm_config(TIM_TypeDef *tim, TIM_Channel_t channel,
                             TIM_PWM_Mode_t mode);

/**
 * @brief  Write the capture/compare register (CCRx) for a channel.
 *         Duty cycle active time = CCRx × tick  (in PWM Mode 1).
 * @param  value  New CCRx value (0 … ARR for 0–100 % duty cycle).
 */
TIM_Status_t tim_set_ccr(TIM_TypeDef *tim, TIM_Channel_t channel,
                          uint32_t value);

/**
 * @brief  Enable a capture/compare channel output (CCER.CCxE = 1).
 *         Must be called BEFORE starting the timer for the first time.
 */
TIM_Status_t tim_channel_enable(TIM_TypeDef *tim, TIM_Channel_t channel);

/**
 * @brief  Disable a capture/compare channel output (CCER.CCxE = 0).
 */
TIM_Status_t tim_channel_disable(TIM_TypeDef *tim, TIM_Channel_t channel);

/* ====================================================================
 * Public API – Advanced Timer (TIM1 / TIM8) Main Output
 * ==================================================================== */

/**
 * @brief  Enable the Main Output Enable (MOE) bit in BDTR.
 *         REQUIRED for TIM1 / TIM8 before any channel output is driven.
 *         Has no meaningful effect on general-purpose timers.
 */
void tim_enable_main_output(TIM_TypeDef *tim);

/** @brief Disable the main output (clear BDTR.MOE). */
void tim_disable_main_output(TIM_TypeDef *tim);

#endif /* TIM_DRIVER_H */
