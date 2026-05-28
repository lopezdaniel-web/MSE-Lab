/**
 * @file    adc_driver.c
 * @brief   Low-level ADC driver implementation (STM32F4xx, CMSIS).
 */

#include "adc_driver.h"

/* ── Private helper: set sample time ────────────────────────────── */

/**
 * @brief  Write sample time bits for a channel into SMPRx.
 *         Channels  0–9  → SMPR2, 3 bits at offset ch*3.
 *         Channels 10–18 → SMPR1, 3 bits at offset (ch-10)*3.
 */
static void set_sample_time(ADC_TypeDef *adc, uint8_t ch, smp_t smp)
{
    uint32_t val = (uint32_t)smp & 0x7U;

    if (ch <= 9U)
    {
        adc->SMPR2 &= ~(0x7U << (ch * 3U));
        adc->SMPR2 |=  (val  << (ch * 3U));
    }
    else if (ch <= 18U)
    {
        uint8_t offset = ch - 10U;
        adc->SMPR1 &= ~(0x7U << (offset * 3U));
        adc->SMPR1 |=  (val  << (offset * 3U));
    }
}

/* ── FR-1: adc_init ──────────────────────────────────────────────── */

/**
 * @brief  Initialise ADC subsystem to default state.
 *
 *  CR1 = 0  →  12-bit resolution (RES[25:24] = 00), scan mode off.
 *  CR2 = 0  →  single conversion, right-aligned, software trigger.
 *  SQR1.L   →  regular sequence length = 1.
 */
adc_status_t adc_init(ADC_TypeDef *adc, adc_cfg_t *cfg)
{
    if ((adc == NULL) || (cfg == NULL)) { return ADC_ERR; }

    /* Enable ADC1 peripheral clock on APB2 */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* ADC prescaler /4 → 4 MHz ADC clock (within 36 MHz spec) */
    ADC1_COMMON->CCR &= ~ADC_CCR_ADCPRE;
    ADC1_COMMON->CCR |=  (1U << ADC_CCR_ADCPRE_Pos);

    adc->CR1  = 0U;             /* 12-bit, no scan                  */
    adc->CR2  = 0U;             /* single, right-align, SW trigger  */
    adc->SQR1 &= ~ADC_SQR1_L;  /* regular sequence length = 1      */

    return ADC_OK;
}

/* ── FR-2: adc_enable ────────────────────────────────────────────── */

/**
 * @brief  Power on the ADC (CR2.ADON = 1).
 *         Caller must insert a stabilisation delay before first conversion.
 */
adc_status_t adc_enable(ADC_TypeDef *adc)
{
    if (adc == NULL) { return ADC_ERR; }
    adc->CR2 |= ADC_CR2_ADON;
    return ADC_OK;
}

/* ── FR-3: adc_set_channel ───────────────────────────────────────── */

/**
 * @brief  Place cfg->ch in slot 1 of the regular sequence (SQR3.SQ1)
 *         and configure its sample time.
 */
adc_status_t adc_set_channel(ADC_TypeDef *adc, adc_cfg_t *cfg)
{
    if ((adc == NULL) || (cfg == NULL) ||
        (cfg->ch > ADC_MAX_CHANNEL)) { return ADC_ERR; }

    adc->SQR3 &= ~ADC_SQR3_SQ1;
    adc->SQR3 |=  ((uint32_t)cfg->ch & 0x1FU);
    set_sample_time(adc, cfg->ch, cfg->smp);

    return ADC_OK;
}

/* ── FR-4: adc_set_injected_channel ─────────────────────────────── */

/**
 * @brief  Configure an injected channel slot in JSQR.
 *         Sets JL = rank-1 and writes the channel to the JSQx field.
 */
adc_status_t adc_set_injected_channel(ADC_TypeDef *adc, adc_cfg_t *cfg)
{
    if ((adc == NULL) || (cfg == NULL) ||
        (cfg->ch > ADC_MAX_CHANNEL) ||
        (cfg->rank < 1U) || (cfg->rank > ADC_MAX_INJECTED_RANK))
    {
        return ADC_ERR;
    }

    /* Set injected sequence length */
    adc->JSQR &= ~ADC_JSQR_JL;
    adc->JSQR |=  (((uint32_t)(cfg->rank - 1U)) << ADC_JSQR_JL_Pos);

    /* Write channel to the JSQx slot for the requested rank */
    uint8_t shift = (cfg->rank - 1U) * 5U;
    adc->JSQR &= ~(0x1FU << shift);
    adc->JSQR |=  ((uint32_t)cfg->ch << shift);

    set_sample_time(adc, cfg->ch, cfg->smp);

    return ADC_OK;
}

/* ── FR-5: adc_start_single ─────────────────────────────────────── */

/**
 * @brief  Trigger one regular conversion via software (CR2.SWSTART).
 *         Clears EOC first so the caller can poll it unambiguously.
 */
adc_status_t adc_start_single(ADC_TypeDef *adc)
{
    if (adc == NULL) { return ADC_ERR; }
    adc->CR2 &= ~ADC_CR2_CONT;    /* single mode   */
    adc->SR  &= ~ADC_SR_EOC;      /* clear EOC     */
    adc->CR2 |=  ADC_CR2_SWSTART; /* start         */
    return ADC_OK;
}

/* ── FR-6: adc_start_continuous / adc_stop_continuous ───────────── */

/**
 * @brief  Enable continuous mode and trigger the first conversion.
 *         ADC restarts automatically after each result is ready in DR.
 */
adc_status_t adc_start_continuous(ADC_TypeDef *adc)
{
    if (adc == NULL) { return ADC_ERR; }
    adc->CR2 |=  ADC_CR2_CONT;
    adc->SR  &= ~ADC_SR_EOC;
    adc->CR2 |=  ADC_CR2_SWSTART;
    return ADC_OK;
}

/** @brief  Stop continuous mode (clear CONT and ADON). */
adc_status_t adc_stop_continuous(ADC_TypeDef *adc)
{
    if (adc == NULL) { return ADC_ERR; }
    adc->CR2 &= ~ADC_CR2_CONT;
    adc->CR2 &= ~ADC_CR2_ADON;
    return ADC_OK;
}

/* ── FR-7: adc_start_injected ────────────────────────────────────── */

/**
 * @brief  Trigger an injected conversion via software (CR2.JSWSTART).
 *         Clears JEOC first for clean polling.
 */
adc_status_t adc_start_injected(ADC_TypeDef *adc)
{
    if (adc == NULL) { return ADC_ERR; }
    adc->SR  &= ~ADC_SR_JEOC;
    adc->CR2 |=  ADC_CR2_JSWSTART;
    return ADC_OK;
}

/* ── FR-8: adc_read_data ─────────────────────────────────────────── */

/**
 * @brief  Poll EOC then return the 12-bit regular conversion result.
 *         Reading DR automatically clears EOC (EOCS = 0, default).
 */
uint16_t adc_read_data(ADC_TypeDef *adc)
{
    if (adc == NULL) { return 0U; }
    while (!(adc->SR & ADC_SR_EOC)) { __NOP(); }
    return (uint16_t)(adc->DR & 0x0FFFU);
}

/* ── FR-9: adc_read_injected_data ────────────────────────────────── */

/**
 * @brief  Poll JEOC then return the result from JDR1–JDR4 for rank.
 *         Clears JEOC manually after reading.
 */
uint16_t adc_read_injected_data(ADC_TypeDef *adc, uint8_t rank)
{
    if ((adc == NULL) || (rank < 1U) || (rank > ADC_MAX_INJECTED_RANK))
    {
        return 0U;
    }

    while (!(adc->SR & ADC_SR_JEOC)) { __NOP(); }

    uint16_t result = 0U;
    switch (rank)
    {
        case 1U: result = (uint16_t)(adc->JDR1 & 0x0FFFU); break;
        case 2U: result = (uint16_t)(adc->JDR2 & 0x0FFFU); break;
        case 3U: result = (uint16_t)(adc->JDR3 & 0x0FFFU); break;
        case 4U: result = (uint16_t)(adc->JDR4 & 0x0FFFU); break;
        default: break;
    }

    adc->SR &= ~ADC_SR_JEOC;
    return result;
}