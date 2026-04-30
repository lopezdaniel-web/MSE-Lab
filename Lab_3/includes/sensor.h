/**
 * @file    sensor.h
 * @brief   Analog Sensor module – potentiometer on PA1 (ADC1 CH1).
 *
 * Hardware mapping (fixed):
 *   ADC     : ADC1
 *   Channel : CH1  →  PA1  (no alternate function needed for analog)
 *   Pin     : PA1  →  configured as GPIO_MODE_ANALOG
 *   Clock   : 16 MHz HSI  →  ADCCLK = 8 MHz (prescaler /2 in ADC_CCR)
 *
 * This module wraps the ADC driver to provide a simple three-function
 * interface: initialise, start, read.
 *
 * Usage:
 *   sensor_init();
 *   sensor_start_conversion();
 *   uint16_t raw = sensor_read_value();   // 0–4095 (12-bit)
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

/**
 * @brief  Initialise the sensor: enable ADC1 clock, configure PA1 as
 *         analog input (no pull), set up ADC1 CH1 with 12-bit resolution
 *         and 84-cycle sample time, then power on the ADC.
 *
 *         Must be called once at startup before any other sensor function.
 */
void sensor_init(void);

/**
 * @brief  Trigger a single ADC conversion on CH1 (software start).
 *         Call sensor_read_value() afterwards to retrieve the result.
 */
void sensor_start_conversion(void);

/**
 * @brief  Poll for end-of-conversion and return the raw 12-bit result.
 *
 *         Blocks until the conversion completes or a timeout expires.
 *         If a timeout or overrun occurs the function returns 0.
 *
 * @return Raw ADC value in the range 0–4095.
 */
uint16_t sensor_read_value(void);

#endif /* SENSOR_H */
