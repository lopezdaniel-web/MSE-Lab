/**
 * @file    mpu6050.h
 * @brief   MPU6050 accelerometer/gyroscope driver.
 *
 * Hardware mapping (I2C1):
 *   PB8  → SCL
 *   PB9  → SDA
 *
 * I2C address: 0x68 (AD0 pin tied to GND)
 */

#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include "stm32f411xe.h"
#include "i2c_driver.h"

/* ── I2C address ─────────────────────────────────────────────────── */
#define MPU6050_ADDR         0x68U

/* ── Register addresses ──────────────────────────────────────────── */
#define MPU6050_PWR_MGMT_1   0x6BU
#define MPU6050_ACCEL_XOUT_H 0x3BU

/* ── Data structure ──────────────────────────────────────────────── */
typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} MPU6050_t;

/* ── Public API ──────────────────────────────────────────────────── */
void mpu6050_init(void);
void mpu6050_config(uint8_t reg, uint8_t value);
void mpu6050_readData(MPU6050_t *data);

#endif /* MPU6050_H */