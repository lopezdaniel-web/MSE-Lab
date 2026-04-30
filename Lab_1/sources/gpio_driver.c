/**
 ******************************************************************************
 * @file    gpio_driver.c
 * @brief   Implementation of the mini-API GPIO using CMSIS registers.
 *
 * All peripheral accesses use the pointers and masks provided by
 * stm32f411xe.h (included through gpio_driver.h → stm32f4xx.h):
 *
 *   GPIOA, GPIOB, GPIOC …    →  GPIO_TypeDef *
 *   RCC->AHB1ENR              →  clock enable register
 *   RCC_AHB1ENR_GPIOAEN       →  mask for GPIOA  (bit 0)
 *   RCC_AHB1ENR_GPIOCEN       →  mask for GPIOC  (bit 2)
 *   …
 ******************************************************************************
 */

#include "gpio_driver.h"

/* ── Helper function: enables AHB1 clock for the given port ───────────── */
static void rcc_gpio_clk_enable(GPIO_TypeDef *GPIOx)
{
    if      (GPIOx == GPIOA) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; }
    else if (GPIOx == GPIOB) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; }
    else if (GPIOx == GPIOC) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; }
    else if (GPIOx == GPIOD) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; }
    else if (GPIOx == GPIOE) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; }
    /* F411RE only exposes up to GPIOE on its physical pins */
}

/* ─────────────────────────────────────────────────────────────────────────── */

void gpio_init(GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t mode, uint32_t pull)
{
    /* 1. Port clock */
    rcc_gpio_clk_enable(GPIOx);

    /* 2. MODER[2*pin+1 : 2*pin] = mode
     *    Example pin=5, mode=OUTPUT(01):
     *      GPIOx->MODER &= ~(11b << 10)   → clears the 2 bits
     *      GPIOx->MODER |=  (01b << 10)   → writes output mode */
    GPIOx->MODER &= ~(3UL << (pin * 2U));
    GPIOx->MODER |=  (mode << (pin * 2U));

    /* 3. PUPDR[2*pin+1 : 2*pin] = pull */
    GPIOx->PUPDR &= ~(3UL << (pin * 2U));
    GPIOx->PUPDR |=  (pull << (pin * 2U));
}

void gpio_write(GPIO_TypeDef *GPIOx, uint32_t pin, uint8_t state)
{
    /* BSRR: bits [15:0]  → individual set   (BS)
     *       bits [31:16] → individual reset (BR)
     * Atomic operation: no need to disable interrupts. */
    if (state) { GPIOx->BSRR =  (1UL << pin);          }   /* set   */
    else       { GPIOx->BSRR =  (1UL << (pin + 16U));  }   /* reset */
}

uint8_t gpio_read(GPIO_TypeDef *GPIOx, uint32_t pin)
{
    return ((GPIOx->IDR & (1UL << pin)) != 0U) ? 1U : 0U;
}

void gpio_toggle(GPIO_TypeDef *GPIOx, uint32_t pin)
{
    /* XOR on ODR: only affects the pin bit */
    GPIOx->ODR ^= (1UL << pin);
}
