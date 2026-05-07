/**
 * @file    adc_driver.h
 * @brief   Low-level ADC driver for STM32F4xx (CMSIS / bare-metal).
 *
 * Supports:
 *  - ADC1 single-channel, single-conversion, polling mode
 *  - 12 / 10 / 8 / 6-bit resolution (CR1.RES)
 *  - Right / left data alignment (CR2.ALIGN)
 *  - Programmable sample time per channel (SMPRx)
 *  - Software-triggered conversion start (CR2.SWSTART)
 *  - End-of-conversion polling via SR.EOC
 *  - Overrun detection via SR.OVR
 *  - Error handling for NULL pointers and invalid channel numbers
 *
 * Peripheral clock must be enabled BEFORE calling adc_init().
 * (The higher-level sensor module handles this.)
 *
 * SRS compliance:
 *   FR-1  Register-level access (ADC_TypeDef from CMSIS)
 *   FR-2  adc_init()          – configure resolution, channel, sample time
 *   FR-3  adc_set_channel()   – select channel in SQR3 and configure SMPRx
 *   FR-4  adc_enable()        – set CR2.ADON and wait for stabilisation
 *   FR-5  adc_disable()       – clear CR2.ADON
 *   FR-6  adc_start()         – trigger conversion via CR2.SWSTART
 *   FR-7  adc_poll_eoc()      – wait for SR.EOC with timeout
 *   FR-8  adc_read()          – read DR and return 16-bit result
 *   FR-9  Error codes         – ADC_Status_t returned by every function
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>   /* NULL */

/* ====================================================================
 * Constants
 * ==================================================================== */

/** @brief ADC stabilisation delay after ADON (minimum ~3 µs at 16 MHz). */
#define ADC_STAB_DELAY_CYCLES   (48U)

/** @brief Default poll timeout (loop iterations before giving up). */
#define ADC_TIMEOUT_CYCLES      (100000U)

/** @brief Maximum valid channel number for ADC1 on STM32F411RE. */
#define ADC_MAX_CHANNEL         (18U)

/* ====================================================================
 * Enumerations
 * ==================================================================== */

/** @brief Conversion resolution (CR1.RES, 2 bits). */
typedef enum
{
    ADC_RES_12BIT = 0x00U,  /**< 12-bit: 0–4095  */
    ADC_RES_10BIT = 0x01U,  /**< 10-bit: 0–1023  */
    ADC_RES_8BIT  = 0x02U,  /**< 8-bit:  0–255   */
    ADC_RES_6BIT  = 0x03U   /**< 6-bit:  0–63    */
} ADC_Resolution_t;

/** @brief Data alignment (CR2.ALIGN). */
typedef enum
{
    ADC_ALIGN_RIGHT = 0x00U, /**< Right-aligned (default)  */
    ADC_ALIGN_LEFT  = 0x01U  /**< Left-aligned             */
} ADC_Align_t;

/**
 * @brief  Sample time selection (SMPRx, 3 bits per channel).
 *         Longer sample time → better accuracy for high-impedance sources.
 */
typedef enum
{
    ADC_SMP_3_CYCLES   = 0x00U,
    ADC_SMP_15_CYCLES  = 0x01U,
    ADC_SMP_28_CYCLES  = 0x02U,
    ADC_SMP_56_CYCLES  = 0x03U,
    ADC_SMP_84_CYCLES  = 0x04U,
    ADC_SMP_112_CYCLES = 0x05U,
    ADC_SMP_144_CYCLES = 0x06U,
    ADC_SMP_480_CYCLES = 0x07U
} ADC_SampleTime_t;

/**
 * @brief  Driver return status (FR-9 – error handling).
 *         Every public function returns one of these codes.
 */
typedef enum
{
    ADC_OK             = 0U, /**< Operation completed successfully     */
    ADC_ERROR_NULL     = 1U, /**< NULL pointer passed as argument      */
    ADC_ERROR_CHANNEL  = 2U, /**< Channel number out of range (> 18)   */
    ADC_ERROR_TIMEOUT  = 3U, /**< EOC did not assert within timeout    */
    ADC_ERROR_OVERRUN  = 4U  /**< SR.OVR set – data was overwritten    */
} ADC_Status_t;

/* ====================================================================
 * Configuration Structure (FR-2)
 * ==================================================================== */

/**
 * @brief  Complete ADC configuration passed to adc_init().
 */
typedef struct
{
    ADC_Resolution_t resolution;  /**< Conversion resolution              */
    ADC_Align_t      align;       /**< Data alignment in DR               */
    uint8_t          channel;     /**< ADC input channel 0–18             */
    ADC_SampleTime_t sampleTime;  /**< Sample-time for the channel        */
} ADC_Config_t;

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief  Initialise the ADC peripheral with the given configuration.
 *         Sets CR1 (resolution), CR2 (alignment, single conversion),
 *         then calls adc_set_channel() to configure SQR3 and SMPRx.
 *         Does NOT enable the ADC; call adc_enable() afterwards.
 *
 * @param  adc     ADC peripheral pointer (ADC1).
 * @param  config  Pointer to the configuration structure.
 * @return ADC_OK, ADC_ERROR_NULL, or ADC_ERROR_CHANNEL.
 */
ADC_Status_t adc_init(ADC_TypeDef *adc, const ADC_Config_t *config);

/**
 * @brief  Select the input channel for the next conversion (FR-3).
 *         Writes channel number into SQR3[4:0] (sequence length = 1)
 *         and programs the sample time in SMPR1 (ch 10–18) or
 *         SMPR2 (ch 0–9).
 *
 * @param  adc      ADC peripheral pointer.
 * @param  channel  Channel 0–18.
 * @param  smp      Sample time selection.
 * @return ADC_OK, ADC_ERROR_NULL, or ADC_ERROR_CHANNEL.
 */
ADC_Status_t adc_set_channel(ADC_TypeDef *adc, uint8_t channel,
                              ADC_SampleTime_t smp);

/**
 * @brief  Enable the ADC (set CR2.ADON) and wait for stabilisation (FR-4).
 *         A short busy-wait loop ensures the ADC is ready before the
 *         first conversion is triggered.
 *
 * @param  adc  ADC peripheral pointer.
 * @return ADC_OK or ADC_ERROR_NULL.
 */
ADC_Status_t adc_enable(ADC_TypeDef *adc);

/**
 * @brief  Disable the ADC (clear CR2.ADON) (FR-5).
 *
 * @param  adc  ADC peripheral pointer.
 * @return ADC_OK or ADC_ERROR_NULL.
 */
ADC_Status_t adc_disable(ADC_TypeDef *adc);

/**
 * @brief  Trigger a software-start conversion (FR-6).
 *         Sets CR2.SWSTART; hardware clears it automatically when the
 *         conversion begins.
 *
 * @param  adc  ADC peripheral pointer.
 * @return ADC_OK or ADC_ERROR_NULL.
 */
ADC_Status_t adc_start(ADC_TypeDef *adc);

/**
 * @brief  Poll SR.EOC until conversion completes or timeout (FR-7).
 *         Also checks SR.OVR on each iteration.
 *
 * @param  adc      ADC peripheral pointer.
 * @param  timeout  Maximum loop iterations to wait.
 * @return ADC_OK, ADC_ERROR_NULL, ADC_ERROR_TIMEOUT, or ADC_ERROR_OVERRUN.
 */
ADC_Status_t adc_poll_eoc(ADC_TypeDef *adc, uint32_t timeout);

/**
 * @brief  Read the 16-bit result from DR (FR-8).
 *         Reading DR also clears SR.EOC automatically (hardware behaviour).
 *
 * @param  adc     ADC peripheral pointer.
 * @param  result  Output pointer; receives the raw ADC value.
 * @return ADC_OK or ADC_ERROR_NULL.
 */
ADC_Status_t adc_read(ADC_TypeDef *adc, uint16_t *result);

#endif /* ADC_DRIVER_H */
