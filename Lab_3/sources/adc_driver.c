/**
 * @file    adc_driver.c
 * @brief   Low-level ADC driver implementation (STM32F4xx, CMSIS).
 *
 * Register map used:
 *
 *   SR    – status register
 *             EOC  [1]  : end-of-conversion flag (set by HW, cleared by DR read)
 *             OVR  [5]  : overrun flag           (set when DR not read in time)
 *
 *   CR1   – control register 1
 *             RES  [25:24] : resolution selection (00=12b, 01=10b, 10=8b, 11=6b)
 *             SCAN [8]     : scan mode (0 = single channel; not used here)
 *
 *   CR2   – control register 2
 *             ADON     [0]  : ADC power-on
 *             CONT     [1]  : continuous mode (0 = single conversion)
 *             ALIGN    [11] : data alignment   (0 = right, 1 = left)
 *             EOCS     [10] : EOC selection    (0 = after sequence, 1 = after each)
 *             SWSTART  [30] : software start of regular conversion
 *
 *   SMPR1 – sample time register 1  (channels 10–18, 3 bits each)
 *   SMPR2 – sample time register 2  (channels  0– 9, 3 bits each)
 *
 *   SQR1  – regular sequence register 1
 *             L[23:20] : sequence length – 1  (0 = one conversion)
 *
 *   SQR3  – regular sequence register 3
 *             SQ1[4:0] : first (and only) channel in the sequence
 *
 *   DR    – regular data register (read clears SR.EOC)
 */

#include "adc_driver.h"

/* ====================================================================
 * Private Helpers
 * ==================================================================== */

/**
 * @brief  Write the 3-bit sample time for a channel into SMPRx.
 *
 *  Channels  0– 9 → SMPR2, field at bits [(ch*3)+2 : ch*3]
 *  Channels 10–18 → SMPR1, field at bits [((ch-10)*3)+2 : (ch-10)*3]
 */
static void set_sample_time(ADC_TypeDef *adc, uint8_t channel,
                             ADC_SampleTime_t smp)
{
    uint32_t smp_val = (uint32_t)smp & 0x7U;

    if (channel <= 9U)
    {
        uint32_t shift = (uint32_t)channel * 3U;
        adc->SMPR2 &= ~(0x7U << shift);
        adc->SMPR2 |=  (smp_val << shift);
    }
    else
    {
        uint32_t shift = ((uint32_t)channel - 10U) * 3U;
        adc->SMPR1 &= ~(0x7U << shift);
        adc->SMPR1 |=  (smp_val << shift);
    }
}

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief  Initialise the ADC with resolution, alignment, channel, and
 *         sample time.
 *
 * CR1 configuration:
 *   - RES[25:24]  → resolution (12 / 10 / 8 / 6 bit)
 *   - SCAN = 0    → single-channel, not scan mode
 *
 * CR2 configuration:
 *   - CONT  = 0   → single conversion (not continuous)
 *   - EOCS  = 1   → EOC set after each individual conversion
 *   - ALIGN[11]   → right or left alignment
 *
 * SQR1:
 *   - L[23:20] = 0 → sequence of exactly 1 conversion
 */
ADC_Status_t adc_init(ADC_TypeDef *adc, const ADC_Config_t *config)
{
    if ((adc == NULL) || (config == NULL)) { return ADC_ERROR_NULL;    }
    if (config->channel > ADC_MAX_CHANNEL) { return ADC_ERROR_CHANNEL; }

    /* ── CR1: resolution (bits 25:24), clear SCAN (bit 8) ─────────── */
    adc->CR1 &= ~(ADC_CR1_RES | ADC_CR1_SCAN);
    adc->CR1 |=  ((uint32_t)config->resolution << ADC_CR1_RES_Pos);

    /* ── CR2: single-conversion, EOCS after each, alignment ─────────  */
    adc->CR2 &= ~(ADC_CR2_CONT | ADC_CR2_ALIGN);
    adc->CR2 |=   ADC_CR2_EOCS;                          /* EOC per conversion */
    if (config->align == ADC_ALIGN_LEFT)
    {
        adc->CR2 |= ADC_CR2_ALIGN;
    }

    /* ── SQR1: sequence length = 1 (L[23:20] = 0) ───────────────────  */
    adc->SQR1 &= ~ADC_SQR1_L;   /* 0000 → 1 conversion */

    /* ── Channel: SQR3 first slot + sample time ─────────────────────  */
    return adc_set_channel(adc, config->channel, config->sampleTime);
}

/**
 * @brief  Select input channel and sample time (FR-3).
 *
 * SQR3 bits [4:0] (SQ1): index of the channel to convert.
 * SMPRx: 3-bit field per channel → see set_sample_time().
 */
ADC_Status_t adc_set_channel(ADC_TypeDef *adc, uint8_t channel,
                              ADC_SampleTime_t smp)
{
    if (adc == NULL)              { return ADC_ERROR_NULL;    }
    if (channel > ADC_MAX_CHANNEL){ return ADC_ERROR_CHANNEL; }

    /* First conversion slot in the regular sequence */
    adc->SQR3 &= ~ADC_SQR3_SQ1;
    adc->SQR3 |=  ((uint32_t)channel & 0x1FU);

    /* Sample time for this channel */
    set_sample_time(adc, channel, smp);

    return ADC_OK;
}

/**
 * @brief  Power on the ADC and wait for stabilisation (FR-4).
 *
 * The ADON bit must be set and a minimum delay must pass (~3 µs at 16 MHz)
 * before triggering the first conversion.  A simple loop is used here
 * (no SysTick dependency).
 */
ADC_Status_t adc_enable(ADC_TypeDef *adc)
{
    volatile uint32_t delay;

    if (adc == NULL) { return ADC_ERROR_NULL; }

    adc->CR2 |= ADC_CR2_ADON;

    /* Stabilisation delay – burns at least ADC_STAB_DELAY_CYCLES cycles */
    for (delay = 0U; delay < ADC_STAB_DELAY_CYCLES; delay++)
    {
        __asm volatile ("nop");
    }

    return ADC_OK;
}

/**
 * @brief  Power off the ADC (FR-5).
 *         Clears CR2.ADON; no conversion can be started while off.
 */
ADC_Status_t adc_disable(ADC_TypeDef *adc)
{
    if (adc == NULL) { return ADC_ERROR_NULL; }

    adc->CR2 &= ~ADC_CR2_ADON;

    return ADC_OK;
}

/**
 * @brief  Software-trigger a regular conversion (FR-6).
 *
 * Writing 1 to CR2.SWSTART begins the conversion; hardware clears
 * SWSTART automatically at the start of conversion.
 * SR.EOC will be set when the result is ready in DR.
 */
ADC_Status_t adc_start(ADC_TypeDef *adc)
{
    if (adc == NULL) { return ADC_ERROR_NULL; }

    adc->CR2 |= ADC_CR2_SWSTART;

    return ADC_OK;
}

/**
 * @brief  Poll SR.EOC with timeout and overrun detection (FR-7).
 *
 * Loop exits when:
 *   a) SR.EOC = 1  → conversion complete, return ADC_OK.
 *   b) SR.OVR = 1  → data overrun (previous result never read), return
 *                     ADC_ERROR_OVERRUN after clearing the flag.
 *   c) timeout counter reaches zero → return ADC_ERROR_TIMEOUT.
 *
 * Note: SR is a read/write-0 register; clearing flags is done by
 * writing 0 to the specific bit (do NOT write 0xFFFFFFFF).
 */
ADC_Status_t adc_poll_eoc(ADC_TypeDef *adc, uint32_t timeout)
{
    if (adc == NULL) { return ADC_ERROR_NULL; }

    while (timeout > 0U)
    {
        /* Check overrun first */
        if (adc->SR & ADC_SR_OVR)
        {
            adc->SR &= ~ADC_SR_OVR;   /* Clear OVR flag */
            return ADC_ERROR_OVERRUN;
        }

        /* End of conversion? */
        if (adc->SR & ADC_SR_EOC)
        {
            return ADC_OK;
        }

        timeout--;
    }

    return ADC_ERROR_TIMEOUT;
}

/**
 * @brief  Read the converted value from DR (FR-8).
 *
 * Reading DR clears SR.EOC automatically (hardware behaviour).
 * The result is right-aligned unless ADC_ALIGN_LEFT was configured.
 * Only the lower 16 bits of DR are used (12-bit resolution at most).
 */
ADC_Status_t adc_read(ADC_TypeDef *adc, uint16_t *result)
{
    if ((adc == NULL) || (result == NULL)) { return ADC_ERROR_NULL; }

    *result = (uint16_t)(adc->DR & 0xFFFFU);

    return ADC_OK;
}
