/**
 * @file    mpu6050.c
 * @brief   MPU6050 driver implementation.
 */

#include "mpu6050.h"

void mpu6050_init(void)
{
    /* ── Enable GPIOB clock ─────────────────────────── */
    /* Same as gpio_initPort(B) → RCC->AHB1ENR |= (1 << 1) */
    RCC->AHB1ENR |= (1U << 1);

    /* ── PB8 – SCL ──────────────────────────────────── */
    /* gpio_setPinMode(B, 8, 2) → MODER = AF (0b10) */
    GPIOB->MODER &= ~(3U << (8 * 2));
    GPIOB->MODER |=  (2U << (8 * 2));

    /* gpio_setAlternateFunction(B, 8, 4) → MODER=AF again + AFR[1] = AF4 */
    GPIOB->AFR[1] &= ~(15U << ((8 - 8) * 4));
    GPIOB->AFR[1] |=  (4U  << ((8 - 8) * 4));

    /* gpio[B]->OTYPER |= (1 << 8) → open-drain */
    GPIOB->OTYPER |= (1U << 8);

    /* pull-up */
    GPIOB->PUPDR &= ~(3U << (8 * 2));
    GPIOB->PUPDR |=  (1U << (8 * 2));

    /* ── PB9 – SDA ──────────────────────────────────── */
    GPIOB->MODER &= ~(3U << (9 * 2));
    GPIOB->MODER |=  (2U << (9 * 2));

    GPIOB->AFR[1] &= ~(15U << ((9 - 8) * 4));
    GPIOB->AFR[1] |=  (4U  << ((9 - 8) * 4));

    GPIOB->OTYPER |= (1U << 9);

    GPIOB->PUPDR &= ~(3U << (9 * 2));
    GPIOB->PUPDR |=  (1U << (9 * 2));

    /* ── I2C init + wake sensor ─────────────────────── */
    i2c_init();

    uint8_t data = 0x00;
    i2c_writeRegDevice(MPU6050_ADDR, MPU6050_PWR_MGMT_1, &data, 1);
}

void mpu6050_config(uint8_t reg, uint8_t value)
{
    i2c_writeRegDevice(MPU6050_ADDR, reg, &value, 1);
}

void mpu6050_readData(MPU6050_t *data)
{
    uint8_t mpu6050_data[14];

    i2c_readRegDevice(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, mpu6050_data, 14);

    data->ax = (int16_t)((mpu6050_data[0]  << 8) | mpu6050_data[1]);
    data->ay = (int16_t)((mpu6050_data[2]  << 8) | mpu6050_data[3]);
    data->az = (int16_t)((mpu6050_data[4]  << 8) | mpu6050_data[5]);

    data->gx = (int16_t)((mpu6050_data[8]  << 8) | mpu6050_data[9]);
    data->gy = (int16_t)((mpu6050_data[10] << 8) | mpu6050_data[11]);
    data->gz = (int16_t)((mpu6050_data[12] << 8) | mpu6050_data[13]);
}