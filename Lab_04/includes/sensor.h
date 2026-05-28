/**
 * @file    sensor.h
 * @brief   Analog sensor module – potentiometer on PA1 (ADC1_IN1).
 *
 * Hardware mapping (fixed):
 *   Pin    : PA1  →  Analog mode (no AF needed for ADC)
 *   ADC    : ADC1, Channel 1
 *   Result : 12-bit (0–4095), right-aligned
 *
 * Usage:
 *   sensor_init();
 *   sensor_start_conversion();
 *   uint16_t raw = sensor_read_value();   // 0–4095
 *   uint8_t  pct = sensor_to_percent(raw); // 0–100
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

/** @brief Initialise PA1 as analog input and configure ADC1 channel 1. */
void     sensor_init(void);

/** @brief Trigger a single ADC conversion (non-blocking start). */
void     sensor_start_conversion(void);

/**
 * @brief  Read the last conversion result (blocks until EOC).
 * @return Raw 12-bit ADC value (0–4095).
 */
uint16_t sensor_read_value(void);

/**
 * @brief  Map a raw 12-bit ADC value to a 0–100 % percentage.
 *         Formula: percent = raw * 100 / 4095
 * @return Duty cycle percentage 0–100.
 */
uint8_t  sensor_to_percent(uint16_t raw);

#endif /* SENSOR_H */