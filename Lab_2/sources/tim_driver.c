/**
 * @file    tim_driver.c
 * @brief   Low-level TIM driver implementation (STM32F4xx, CMSIS).
 *
 * Register map used:
 *
 *   CR1    – control register 1
 *              CEN  [0]   : counter enable
 *              ARPE [7]   : auto-reload preload enable
 *   EGR    – event generation register
 *              UG   [0]   : update generation (forces PSC/ARR shadow load)
 *   SR     – status register
 *              UIF  [0]   : update interrupt flag (set on overflow)
 *   PSC    – prescaler         (16-bit, shadow)
 *   ARR    – auto-reload value (16 or 32-bit depending on timer, shadow)
 *   CNT    – counter value     (16 or 32-bit depending on timer)
 *   CCMRx  – capture/compare mode registers
 *              CCxS [1:0]   : channel direction (00 = output)
 *              OCxPE [3]    : output compare preload enable
 *              OCxM  [6:4]  : output compare mode (110 = PWM1, 111 = PWM2)
 *   CCER   – capture/compare enable register
 *              CCxE [4n]  : channel output enable
 *   CCRx   – capture/compare value (duty cycle control)
 *   BDTR   – break and dead-time register (TIM1 / TIM8 only)
 *              MOE  [15]  : main output enable (REQUIRED for TIM1 output)
 */

#include "tim_driver.h"

/* ====================================================================
 * Public API – Core Timer
 * ==================================================================== */

/**
 * @brief  Initialise a timer: set PSC, ARR, reset CNT, force shadow
 *         register update via EGR.UG, then clear UIF.
 *
 * Why EGR.UG?
 *   PSC and ARR have shadow registers.  Writing PSC/ARR only updates the
 *   preload registers; the hardware copies them to the active registers on
 *   the next Update Event (UEV).  Writing EGR.UG forces an immediate UEV
 *   so the new values take effect before the first enable.
 *
 * Why clear UIF after EGR.UG?
 *   The forced UEV sets UIF = 1.  Clearing it here prevents a spurious
 *   "overflow detected" on the very first poll in timer_delay_ms().
 */
TIM_Status_t tim_init(TIM_TypeDef *tim, uint16_t psc, uint32_t arr)
{
    if (tim == NULL) { return TIM_ERROR; }

    tim->CR1 &= ~TIM_CR1_CEN;  /* Stop counter while reconfiguring   */
    tim->PSC  = (uint32_t)psc; /* Prescaler preload                  */
    tim->ARR  = arr;            /* Auto-reload preload                */
    tim->CNT  = 0U;             /* Reset counter                      */
    tim->EGR  = TIM_EGR_UG;    /* Force UEV → copies PSC/ARR shadows */
    tim->SR  &= ~TIM_SR_UIF;   /* Clear the UIF set by the forced UEV */

    return TIM_OK;
}

/** @brief Start the timer counter (CR1.CEN = 1). */
void tim_enable(TIM_TypeDef *tim)
{
    tim->CR1 |= TIM_CR1_CEN;
}

/** @brief Stop the timer counter (CR1.CEN = 0). */
void tim_disable(TIM_TypeDef *tim)
{
    tim->CR1 &= ~TIM_CR1_CEN;
}

/** @brief Set the counter register to zero. */
void tim_reset_counter(TIM_TypeDef *tim)
{
    tim->CNT = 0U;
}

/** @brief Return the raw counter register value. */
uint32_t tim_get_counter(TIM_TypeDef *tim)
{
    return tim->CNT;
}

/* ====================================================================
 * Public API – Status Flags
 * ==================================================================== */

/**
 * @brief  Read SR.UIF (set by hardware on each counter overflow).
 * @return 1 if an overflow has occurred since the last clear, else 0.
 */
uint8_t tim_get_update_flag(TIM_TypeDef *tim)
{
    return (uint8_t)((tim->SR & TIM_SR_UIF) ? 1U : 0U);
}

/** @brief  Clear SR.UIF (write 0; this register is rc_w0). */
void tim_clear_update_flag(TIM_TypeDef *tim)
{
    tim->SR &= ~TIM_SR_UIF;
}

/* ====================================================================
 * Public API – PWM Configuration
 * ==================================================================== */

/**
 * @brief  Configure a capture/compare channel in PWM output mode.
 *
 * Per channel, the following CCMR bits are modified:
 *
 *  CH1/CH3 → CCMR1/CCMR2 lower byte:
 *    Bits [1:0] CCxS   = 00  (output compare, not input capture)
 *    Bits [6:4] OCxM   = mode (6 = PWM1, 7 = PWM2)
 *    Bit  [3]   OCxPE  = 1   (preload: CCRx loaded on UEV only)
 *
 *  CH2/CH4 → CCMR1/CCMR2 upper byte (same fields, shifted by 8):
 *    Bits [9:8]  CC2S/CC4S = 00
 *    Bits [14:12] OC2M/OC4M = mode
 *    Bit  [11]   OC2PE/OC4PE = 1
 *
 *  CR1.ARPE = 1  →  ARR also has preload (consistent with OC preload).
 */
TIM_Status_t tim_pwm_config(TIM_TypeDef *tim, TIM_Channel_t channel,
                             TIM_PWM_Mode_t mode)
{
    if (tim == NULL) { return TIM_ERROR; }

    uint32_t ocm = (uint32_t)mode; /* 6 or 7 */

    switch (channel)
    {
        /* ---- Channel 1 (CCMR1 bits [6:0]) --------------------------- */
        case TIM_CHANNEL_1:
            tim->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S);
            tim->CCMR1 |=  (ocm << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
            break;

        /* ---- Channel 2 (CCMR1 bits [14:8]) -------------------------- */
        case TIM_CHANNEL_2:
            tim->CCMR1 &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_CC2S);
            tim->CCMR1 |=  (ocm << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
            break;

        /* ---- Channel 3 (CCMR2 bits [6:0]) --------------------------- */
        case TIM_CHANNEL_3:
            tim->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_CC3S);
            tim->CCMR2 |=  (ocm << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;
            break;

        /* ---- Channel 4 (CCMR2 bits [14:8]) -------------------------- */
        case TIM_CHANNEL_4:
            tim->CCMR2 &= ~(TIM_CCMR2_OC4M | TIM_CCMR2_CC4S);
            tim->CCMR2 |=  (ocm << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;
            break;

        default:
            return TIM_ERROR;
    }

    /* Enable auto-reload preload so ARR changes are also double-buffered */
    tim->CR1 |= TIM_CR1_ARPE;

    return TIM_OK;
}

/**
 * @brief  Write the capture/compare register for a channel.
 *
 *  In PWM Mode 1:  output = HIGH while CNT < CCRx
 *                         = LOW  while CNT >= CCRx
 *  Duty cycle (%) = CCRx / (ARR + 1) × 100
 *
 *  Because OC1PE is set, the new value is latched into the shadow
 *  register and becomes active on the next Update Event (UEV).
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

/**
 * @brief  Enable the channel output pin (CCER.CCxE = 1).
 *
 *  CCER bit layout:
 *    CH1: bit  0  (CC1E)
 *    CH2: bit  4  (CC2E)
 *    CH3: bit  8  (CC3E)
 *    CH4: bit 12  (CC4E)
 */
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

/** @brief  Disable a channel output pin (CCER.CCxE = 0). */
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

/* ====================================================================
 * Public API – Advanced Timer Main Output (TIM1 / TIM8)
 * ==================================================================== */

/**
 * @brief  Set BDTR.MOE (Main Output Enable) for TIM1 / TIM8.
 *
 * Why needed?
 *   TIM1 and TIM8 are "advanced-control" timers.  Unlike general-purpose
 *   timers, their channel outputs are gated by BDTR.MOE.  Even with CCxE=1
 *   and the counter running, no signal appears on the pin until MOE is set.
 *
 * BDTR.MOE is bit 15.
 */
void tim_enable_main_output(TIM_TypeDef *tim)
{
    tim->BDTR |= TIM_BDTR_MOE;
}

/** @brief  Clear BDTR.MOE – disables all channel outputs of TIM1/TIM8. */
void tim_disable_main_output(TIM_TypeDef *tim)
{
    tim->BDTR &= ~TIM_BDTR_MOE;
}
