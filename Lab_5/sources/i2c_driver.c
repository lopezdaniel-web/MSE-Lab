/**
 * @file    i2c_driver.c
 * @brief   I2C1 peripheral driver for STM32F411.
 */

#include "i2c_driver.h"

uint32_t i2c_calc_ccr(uint32_t scl_freq_hz)
{
    return SystemCoreClock / (2 * scl_freq_hz);
}

uint32_t i2c_trise(uint32_t scl_freq_hz)
{
    uint32_t max_rise_ns;

    if (scl_freq_hz <= 100000)
        max_rise_ns = 1000;
    else if (scl_freq_hz <= 400000)
        max_rise_ns = 300;
    else
        max_rise_ns = 120;

    uint32_t pclk_mhz = SystemCoreClock / 1000000UL;
    uint32_t trise    = ((max_rise_ns * pclk_mhz) / 1000UL) + 1UL;
    return trise;
}

void i2c_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    I2C1->CR1 |=  I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    I2C1->CR2 = 0;
    I2C1->CR2 |= (16U & 0x3F);

    I2C1->CCR   = i2c_calc_ccr(SLC_FREQ);
    I2C1->TRISE = i2c_trise(SLC_FREQ);

    I2C1->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN);

    I2C1->CR1 |= I2C_CR1_PE;
}

#define I2C_TIMEOUT 10000U
#define I2C_WAIT(cond) do {                                             \
    volatile uint32_t _t = 0;                                           \
    while (!(cond) && _t < I2C_TIMEOUT) _t++;                          \
    if (_t >= I2C_TIMEOUT) { I2C1->CR1 |= I2C_CR1_STOP; return; }     \
} while(0)

void i2c_writeRegDevice(uint8_t device_address, uint8_t register_address,
                        uint8_t *data, uint32_t len)
{
    for (volatile uint32_t t = 0;
         (I2C1->SR2 & I2C_SR2_BUSY) && t < I2C_TIMEOUT; t++);
    if (I2C1->SR2 & I2C_SR2_BUSY) return;

    I2C1->CR1 |= I2C_CR1_START;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_SB);

    I2C1->DR = (device_address << 1);
    I2C_WAIT(I2C1->SR1 & I2C_SR1_ADDR);

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = register_address;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_TXE);

    for (uint32_t i = 0; i < len; i++)
    {
        I2C1->DR = data[i];
        I2C_WAIT(I2C1->SR1 & I2C_SR1_TXE);
    }

    I2C_WAIT(I2C1->SR1 & I2C_SR1_BTF);
    I2C1->CR1 |= I2C_CR1_STOP;
}

void i2c_writeDevice(uint8_t device_address, uint8_t *data, uint32_t len)
{
    I2C1->CR1 |= I2C_CR1_START;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_SB);

    I2C1->DR = (device_address << 1);
    I2C_WAIT(I2C1->SR1 & I2C_SR1_ADDR);

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint32_t i = 0; i < len; i++)
    {
        I2C1->DR = data[i];
        I2C_WAIT(I2C1->SR1 & I2C_SR1_TXE);
    }

    I2C1->CR1 |= I2C_CR1_STOP;
}

void i2c_readRegDevice(uint8_t device_address, uint8_t register_address,
                       uint8_t *data, uint32_t len)
{
    for (volatile uint32_t t = 0;
         (I2C1->SR2 & I2C_SR2_BUSY) && t < 5000; t++);
    if (I2C1->SR2 & I2C_SR2_BUSY) return;

    I2C1->CR1 |= I2C_CR1_START;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_SB);

    I2C1->DR = (device_address << 1);
    I2C_WAIT(I2C1->SR1 & I2C_SR1_ADDR);

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = register_address;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_TXE);

    I2C1->CR1 |= I2C_CR1_START;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_SB);

    I2C1->DR = (device_address << 1) | 1;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_ADDR);

    I2C1->CR1 |= I2C_CR1_ACK;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint32_t i = 0; i < len; i++)
    {
        if (i == (len - 1))
        {
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |=  I2C_CR1_STOP;
        }

        I2C_WAIT(I2C1->SR1 & I2C_SR1_RXNE);
        data[i] = I2C1->DR;
    }
}

void i2c_readDevice(uint8_t device_address, uint8_t *data, uint32_t len)
{
    I2C1->CR1 |= I2C_CR1_START;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_SB);

    I2C1->DR = (device_address << 1);
    I2C_WAIT(I2C1->SR1 & I2C_SR1_ADDR);
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->CR1 |= I2C_CR1_START;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_SB);

    I2C1->DR = (device_address << 1) | 1;
    I2C_WAIT(I2C1->SR1 & I2C_SR1_ADDR);
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->CR1 |= I2C_CR1_ACK;

    for (uint32_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |=  I2C_CR1_STOP;
        }
        I2C_WAIT(I2C1->SR1 & I2C_SR1_RXNE);
        data[i] = I2C1->DR;
    }
}