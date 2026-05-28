/**
 * @file    adc_driver.h
 * @brief   Low-level ADC driver for STM32F4xx (CMSIS / bare-metal).
 *
 * Resolution: 12-bit by default (CR1.RES = 00 after reset, not configured).
 * Alignment : right-aligned by default (CR2.ALIGN = 0 after reset).
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>

/* ── Constants ───────────────────────────────────────────────────── */

#define ADC_MAX_CHANNEL       (18U)
#define ADC_MAX_INJECTED_RANK (4U)
#define ADC_FULL_SCALE        (4095U)   /* 12-bit max value */

/* ── Types ───────────────────────────────────────────────────────── */

typedef enum { ADC_OK = 0U, ADC_ERR = 1U } adc_status_t;

typedef enum
{
    SMP_3   = 0U,
    SMP_15  = 1U,
    SMP_28  = 2U,
    SMP_56  = 3U,
    SMP_84  = 4U,   /* recommended for high-impedance sources */
    SMP_112 = 5U,
    SMP_144 = 6U,
    SMP_480 = 7U
} smp_t;

typedef struct
{
    uint8_t ch;     /* channel number 0–18              */
    smp_t   smp;    /* sample time selection            */
    uint8_t rank;   /* injected rank 1–4, 0 = not used  */
} adc_cfg_t;

adc_status_t adc_init(ADC_TypeDef *adc, adc_cfg_t *cfg);
adc_status_t adc_enable(ADC_TypeDef *adc);
adc_status_t adc_set_channel(ADC_TypeDef *adc, adc_cfg_t *cfg);
adc_status_t adc_set_injected_channel(ADC_TypeDef *adc, adc_cfg_t *cfg);
adc_status_t adc_start_single(ADC_TypeDef *adc);
adc_status_t adc_start_continuous(ADC_TypeDef *adc);
adc_status_t adc_stop_continuous(ADC_TypeDef *adc);
adc_status_t adc_start_injected(ADC_TypeDef *adc);
uint16_t     adc_read_data(ADC_TypeDef *adc);
uint16_t     adc_read_injected_data(ADC_TypeDef *adc, uint8_t rank);

#endif /* ADC_DRIVER_H */