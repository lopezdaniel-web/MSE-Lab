/**
 ******************************************************************************
 * @file    gpio_driver.h
 * @brief   Thin GPIO driver for STM32F411RE.
 *
 * With CMSIS integrated, this header NO LONGER needs to define GPIO_TypeDef,
 * RCC_TypeDef, or base addresses: stm32f411xe.h provides all of them.
 *
 * What lives here:
 *   - Readable mode/pull constants (MODE_INPUT, PULLUP, …)
 *   - Mini-API prototypes that abstract bit manipulation.
 *
 * Direct usage (CMSIS style, without going through the API):
 *   GPIOA->ODR    ^= (1UL << 5U);          // toggle PA5
 *   GPIOC->IDR    &  (1UL << 13U);         // read PC13
 *   GPIOB->BSRR   =  (1UL << 7U);          // set   PB7
 *   GPIOB->BSRR   =  (1UL << (7U + 16U));  // reset PB7
 ******************************************************************************
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdint.h>
#include "stm32f4xx.h"   /* GPIO_TypeDef, GPIOA..GPIOH, RCC, RCC_AHB1ENR_GPIOxEN */

/* ── Mode constants (MODER field, 2 bits per pin) ──────────────────── */
#define GPIO_MODE_INPUT     0x00U   /**< Pin as digital input                */
#define GPIO_MODE_OUTPUT    0x01U   /**< Pin as general purpose output       */
#define GPIO_MODE_AF        0x02U   /**< Alternate function (UART, SPI, …)  */
#define GPIO_MODE_ANALOG    0x03U   /**< Analog mode (ADC/DAC)              */

/* ── Pull-up/down constants (PUPDR field, 2 bits per pin) ─────────────── */
#define GPIO_NOPULL         0x00U   /**< No internal resistor               */
#define GPIO_PULLUP         0x01U   /**< Internal pull-up                   */
#define GPIO_PULLDOWN       0x02U   /**< Internal pull-down                 */

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief  Enables port clock and configures mode and pull for a pin.
 * @param  GPIOx  CMSIS pointer to port: GPIOA, GPIOB, GPIOC, …
 * @param  pin    Pin number (0–15).
 * @param  mode   GPIO_MODE_INPUT | GPIO_MODE_OUTPUT | …
 * @param  pull   GPIO_NOPULL | GPIO_PULLUP | GPIO_PULLDOWN
 */
void    gpio_init   (GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t mode, uint32_t pull);

/**
 * @brief  Writes a logical state to an output pin using BSRR
 *         (atomic operation, no read-modify-write).
 */
void    gpio_write  (GPIO_TypeDef *GPIOx, uint32_t pin, uint8_t state);

/**
 * @brief  Reads logical state of pin from IDR.
 * @return 1 if pin is HIGH, 0 if pin is LOW.
 */
uint8_t gpio_read   (GPIO_TypeDef *GPIOx, uint32_t pin);

/**
 * @brief  Toggles current pin state via XOR on ODR.
 */
void    gpio_toggle (GPIO_TypeDef *GPIOx, uint32_t pin);

#endif /* GPIO_DRIVER_H */
