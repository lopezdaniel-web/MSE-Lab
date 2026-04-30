/**
 * @file    sensor.c
 * @brief   Analog Sensor module implementation – ADC1 CH1 on PA1.
 *
 * ── Hardware connections ─────────────────────────────────────────────
 *
 *   Potentiometer wiper → PA1  (CN8 pin 2 on NUCLEO-64 A1 connector)
 *   PA1 must NOT have any pull-up/pull-down; GPIO_MODE_ANALOG disables
 *   the Schmitt trigger and internal resistors automatically.
 *
 * ── Clock derivation ─────────────────────────────────────────────────
 *
 *   SYSCLK  = 16 MHz  (HSI, no PLL)
 *   APB2    = 16 MHz  (PPRE2 = /1 after reset)
 *   ADCCLK  =  8 MHz  (ADC prescaler /2 via ADC_CCR.ADCPRE = 00)
 *
 * ── Timing budget for one conversion (12-bit, 84-cycle sample time) ──
 *
 *   Sample phase  =  84 cycles
 *   Conversion    =  12 cycles   (SAR approximation)
 *   Total         =  96 cycles at 8 MHz ≈ 12 µs per conversion
 *
 * ── Module dependencies ───────────────────────────────────────────────
 *
 *   adc_driver  – all register-level ADC operations
 *   gpio_driver – PA1 analog mode configuration
 */

#include "sensor.h"
#include "adc_driver.h"
#include "gpio_driver.h"
#include "stm32f411xe.h"

/* ====================================================================
 * Private Constants
 * ==================================================================== */

#define SENSOR_ADC          ADC1            /**< ADC instance to use      */
#define SENSOR_CHANNEL      (1U)            /**< ADC1 CH1 = PA1           */
#define SENSOR_GPIO_PORT    GPIOA           /**< GPIO port for PA1        */
#define SENSOR_GPIO_PIN     (1U)            /**< Pin number               */

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief  Initialise the analog sensor channel.
 *
 * Steps:
 *  1. Enable GPIOA AHB1 clock and configure PA1 as analog (no pull).
 *  2. Enable ADC1 APB2 clock.
 *  3. Set ADC common prescaler to /2 (ADCCLK = 8 MHz).
 *  4. Call adc_init() with 12-bit right-aligned, CH1, 84 cycles sample.
 *  5. Call adc_enable() to power on and stabilise the ADC.
 */
void sensor_init(void)
{
    ADC_Config_t cfg;

    /* ── 1. Configure PA1 as analog input ──────────────────────────── */
    /*
     * gpio_init() with GPIO_MODE_ANALOG:
     *   - MODER[3:2] = 11  → analog mode
     *   - Schmitt trigger disabled automatically by hardware
     *   - Internal pull-up/down disabled (PUPDR = 00 is default after reset)
     * gpio_set_pupd() called explicitly for clarity.
     */
    gpio_init(SENSOR_GPIO_PORT, SENSOR_GPIO_PIN, GPIO_MODE_ANALOG);
    gpio_set_pupd(SENSOR_GPIO_PORT, SENSOR_GPIO_PIN, GPIO_PUPD_NONE);

    /* ── 2. Enable ADC1 peripheral clock (APB2) ─────────────────────── */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* ── 3. Set ADC common clock prescaler /2 (ADCCLK = 8 MHz) ──────── */
    /*
     * ADC_CCR.ADCPRE[17:16]:
     *   00 → /2   (ADCCLK = PCLK2 / 2 = 8 MHz)
     * Must be ≤ 36 MHz; 8 MHz satisfies all sample-time requirements.
     */
    ADC->CCR &= ~ADC_CCR_ADCPRE;   /* 00 = /2 (default, written for clarity) */

    /* ── 4. Initialise ADC1: 12-bit, right-aligned, CH1, 84 cycles ──── */
    cfg.resolution = ADC_RES_12BIT;
    cfg.align      = ADC_ALIGN_RIGHT;
    cfg.channel    = SENSOR_CHANNEL;
    cfg.sampleTime = ADC_SMP_84_CYCLES;

    adc_init(SENSOR_ADC, &cfg);

    /* ── 5. Power on ADC and wait for stabilisation ──────────────────── */
    adc_enable(SENSOR_ADC);
}

/**
 * @brief  Start a single software-triggered conversion on CH1.
 *
 * Internally calls adc_start() which sets CR2.SWSTART.
 * The conversion runs autonomously; poll for completion via
 * sensor_read_value().
 */
void sensor_start_conversion(void)
{
    adc_start(SENSOR_ADC);
}

/**
 * @brief  Wait for conversion to finish and return the raw 12-bit result.
 *
 * Calls adc_poll_eoc() with ADC_TIMEOUT_CYCLES; if the conversion does
 * not complete (timeout or overrun), returns 0 to signal an error to
 * the caller.
 *
 * @return Raw ADC value 0–4095, or 0 on error.
 */
uint16_t sensor_read_value(void)
{
    uint16_t     result = 0U;
    ADC_Status_t status;

    status = adc_poll_eoc(SENSOR_ADC, ADC_TIMEOUT_CYCLES);

    if (status == ADC_OK)
    {
        adc_read(SENSOR_ADC, &result);
    }

    return result;
}
