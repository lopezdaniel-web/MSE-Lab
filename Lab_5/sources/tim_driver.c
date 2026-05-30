/**
 * @file    tim_driver.c
 * @brief   Low-level TIM driver implementation (STM32F4xx, CMSIS).
 *
 */

#include "tim_driver.h"

/* ====================================================================
 * Core Timer Control
 * ==================================================================== */

/**
 * @brief  Initialise PSC and ARR, reset counter, force shadow load.
 *         Timer is left DISABLED after this call.
 */
TIM_Status_t tim_init(TIM_TypeDef *tim, uint16_t psc, uint32_t arr)
{
    if (tim == NULL) { return TIM_ERROR; }

    tim->CR1 &= ~TIM_CR1_CEN;   /* Stop counter                     */
    tim->PSC  = (uint32_t)psc;  /* Prescaler                        */
    tim->ARR  = arr;            /* Auto-reload                      */
    tim->CNT  = 0U;             /* Reset counter                    */
    tim->EGR  = TIM_EGR_UG;     /* Force UEV → load PSC/ARR shadows */
    tim->SR  &= ~TIM_SR_UIF;    /* Clear UIF set by forced UEV      */

    return TIM_OK;
}

void tim_enable(TIM_TypeDef *tim)         { tim->CR1 |=  TIM_CR1_CEN; }
void tim_disable(TIM_TypeDef *tim)        { tim->CR1 &= ~TIM_CR1_CEN; }
void tim_reset_counter(TIM_TypeDef *tim)  { tim->CNT  = 0U;            }
uint32_t tim_get_counter(TIM_TypeDef *tim){ return tim->CNT;            }
uint32_t tim_get_arr(TIM_TypeDef *tim)    { return tim->ARR;            }

/* ====================================================================
 * Status Flags
 * ==================================================================== */

uint8_t tim_get_update_flag(TIM_TypeDef *tim)
{
    return (uint8_t)((tim->SR & TIM_SR_UIF) ? 1U : 0U);
}

void tim_clear_update_flag(TIM_TypeDef *tim)
{
    tim->SR &= ~TIM_SR_UIF;
}

/* ====================================================================
 * Frequency Configuration
 * ==================================================================== */

/**
 * @brief  Configure PSC and ARR to produce the requested overflow frequency.
 *
 * Mirrors the working implementation's two-range strategy:
 *
 *   freq >= 1000 Hz  →  PSC = 0   (full 16 MHz resolution)
 *                        ARR = (16 000 000 / freq) - 1
 *
 *   freq <  1000 Hz  →  PSC = 15999  (1 kHz tick)
 *                        ARR = (1000 / freq) - 1
 *
 * After writing PSC/ARR the function forces a UEV (EGR.UG) so the
 * new values are active immediately — same as tim_init().
 */
TIM_Status_t tim_set_frequency(TIM_TypeDef *tim, uint32_t freq_hz)
{
    if ((tim == NULL) || (freq_hz == 0U)) { return TIM_ERROR; }

    uint32_t psc, arr;

    if (freq_hz >= 1000U)
    {
        psc = 0U;
        arr = (TIM_CLOCK_HZ / freq_hz) - 1U;
    }
    else
    {
        psc = 15999U;
        arr = ((TIM_CLOCK_HZ / (psc + 1U)) / freq_hz) - 1U;
    }

    /* Clamp ARR to 16-bit for timers that are 16-bit (TIM1/TIM3/TIM4/TIM9-11).
       TIM2 and TIM5 are 32-bit so the clamp has no effect there.        */
    if (arr > 65535U) { arr = 65535U; }
    if (arr < 1U)     { arr = 1U;     }

    tim->CR1 &= ~TIM_CR1_CEN;
    tim->PSC  = psc;
    tim->ARR  = arr;
    tim->CNT  = 0U;
    tim->EGR  = TIM_EGR_UG;
    tim->SR  &= ~TIM_SR_UIF;

    return TIM_OK;
}

/* ====================================================================
 * PWM Configuration  (NO OC preload — CCR is active immediately)
 * ==================================================================== */

/**
 * @brief  Configure a channel in PWM output mode.
 *
 * CCxS = 00 → output compare (not input capture)
 * OCxM = mode (6 = PWM Mode 1, 7 = PWM Mode 2)
 *
 * OC1PE is intentionally NOT set, so CCR writes take effect on
 * the very next counter cycle without requiring a UEV flush.
 */
TIM_Status_t tim_pwm_config(TIM_TypeDef *tim, TIM_Channel_t channel,
                             TIM_PWM_Mode_t mode)
{
    if (tim == NULL) { return TIM_ERROR; }

    uint32_t ocm = (uint32_t)mode;

    switch (channel)
    {
        case TIM_CHANNEL_1:
            tim->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S);
            tim->CCMR1 |=  (ocm << TIM_CCMR1_OC1M_Pos);
            break;

        case TIM_CHANNEL_2:
            tim->CCMR1 &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_CC2S);
            tim->CCMR1 |=  (ocm << TIM_CCMR1_OC2M_Pos);
            break;

        case TIM_CHANNEL_3:
            tim->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_CC3S);
            tim->CCMR2 |=  (ocm << TIM_CCMR2_OC3M_Pos);
            break;

        case TIM_CHANNEL_4:
            tim->CCMR2 &= ~(TIM_CCMR2_OC4M | TIM_CCMR2_CC4S);
            tim->CCMR2 |=  (ocm << TIM_CCMR2_OC4M_Pos);
            break;

        default: return TIM_ERROR;
    }

    return TIM_OK;
}

/**
 * @brief  Write the CCRx register for a channel.
 *         Because OC preload is disabled, this takes effect immediately.
 */
TIM_Status_t tim_set_ccr(TIM_TypeDef *tim, TIM_Channel_t channel,
                          uint32_t value)
{
    if (tim == NULL) { return TIM_ERROR; }

    switch (channel)
    {
        case TIM_CHANNEL_1: tim->CCR1 = value; break;
        case TIM_CHANNEL_2: tim->CCR2 = value; break;
        case TIM_CHANNEL_3: tim->CCR3 = value; break;
        case TIM_CHANNEL_4: tim->CCR4 = value; break;
        default: return TIM_ERROR;
    }

    return TIM_OK;
}

/* ====================================================================
 * Channel Output Enable / Disable  (CCER)
 * ==================================================================== */

TIM_Status_t tim_channel_enable(TIM_TypeDef *tim, TIM_Channel_t channel)
{
    if (tim == NULL) { return TIM_ERROR; }
    switch (channel)
    {
        case TIM_CHANNEL_1: tim->CCER |= TIM_CCER_CC1E; break;
        case TIM_CHANNEL_2: tim->CCER |= TIM_CCER_CC2E; break;
        case TIM_CHANNEL_3: tim->CCER |= TIM_CCER_CC3E; break;
        case TIM_CHANNEL_4: tim->CCER |= TIM_CCER_CC4E; break;
        default: return TIM_ERROR;
    }
    return TIM_OK;
}

TIM_Status_t tim_channel_disable(TIM_TypeDef *tim, TIM_Channel_t channel)
{
    if (tim == NULL) { return TIM_ERROR; }
    switch (channel)
    {
        case TIM_CHANNEL_1: tim->CCER &= ~TIM_CCER_CC1E; break;
        case TIM_CHANNEL_2: tim->CCER &= ~TIM_CCER_CC2E; break;
        case TIM_CHANNEL_3: tim->CCER &= ~TIM_CCER_CC3E; break;
        case TIM_CHANNEL_4: tim->CCER &= ~TIM_CCER_CC4E; break;
        default: return TIM_ERROR;
    }
    return TIM_OK;
}