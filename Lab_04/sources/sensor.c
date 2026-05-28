/**
 * @file    sensor.c
 * @brief   Analog sensor module – potentiometer/sensor on PA1 (ADC1_IN1).
 *
 * Why PA1?
 *   PA0 is already occupied by TIM2_CH1 (PWM output from Lab 02).
 *   PA1 = ADC1_IN1, available on CN8 pin 2 of the NUCLEO-F411RE.
 *
 * Initialisation sequence:
 *   1. Enable GPIOA clock and configure PA1 as analog (MODER = 11).
 *      No pull-up/pull-down — analog mode floats internally.
 *   2. Call adc_init(ADC1)  → clock, prescaler, CR1/CR2 defaults.
 *   3. Call adc_set_channel  → ADC1_IN1, 84-cycle sample time
 *      (recommended for high-impedance sources like potentiometers).
 *   4. Call adc_enable(ADC1) → ADON = 1.
 *   5. Insert stabilisation delay (few NOPs at 16 MHz ≈ tens of µs).
 */

#include "sensor.h"
#include "adc_driver.h"
#include "gpio_driver.h"
#include "stm32f411xe.h"

#define SENSOR_PORT     GPIOA
#define SENSOR_PIN      (1U)    /* PA1 = ADC1_IN1, CN8 pin 2 */
#define SENSOR_CHANNEL  (1U)
#define SENSOR_ADC      ADC1

/* ── sensor_init ─────────────────────────────────────────────────── */
void sensor_init(void)
{
    adc_cfg_t cfg = { .ch = 1U, .smp = SMP_84, .rank = 0U };

    gpio_init(SENSOR_PORT, SENSOR_PIN, GPIO_MODE_ANALOG);
    gpio_set_pupd(SENSOR_PORT, SENSOR_PIN, GPIO_PUPD_NONE);
    
    /* 1. GPIO – PA1 analog */
    adc_init(SENSOR_ADC, &cfg);
    adc_set_channel(SENSOR_ADC, &cfg);
    adc_enable(SENSOR_ADC);
    
    /* 5. Stabilisation delay */
    for (volatile uint32_t i = 0U; i < 100U; i++);
}

/* ── sensor_start_conversion ────────────────────────────────────── */
/**
 * @brief  Trigger one single ADC conversion on channel 1.
 *         Returns immediately; call sensor_read_value() to get result.
 */
void sensor_start_conversion(void)
{
    adc_start_single(SENSOR_ADC);
}

/* ── sensor_read_value ───────────────────────────────────────────── */
/**
 * @brief  Wait for conversion to complete and return the raw 12-bit value.
 * @return 0–4095.
 */
uint16_t sensor_read_value(void)
{
    return adc_read_data(SENSOR_ADC);
}

/* ── sensor_to_percent ───────────────────────────────────────────── */
/**
 * @brief  Map 0–4095 to 0–100 %.
 *         Uses integer arithmetic: (raw * 100) / 4095.
 */
uint8_t sensor_to_percent(uint16_t raw)
{
    return (uint8_t)(((uint32_t)raw * 100U) / ADC_FULL_SCALE);
}