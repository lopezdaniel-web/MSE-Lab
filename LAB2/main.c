/**
 * @file    gpio_driver.h
 * @brief   Low-level GPIO driver for STM32F4xx (CMSIS / bare-metal).
 *
 * Supports:
 *  - Input / Output / Alternate-Function / Analog modes
 *  - Output type (push-pull / open-drain)
 *  - Output speed configuration
 *  - Internal pull-up / pull-down
 *  - Alternate function (AFR) configuration for AF0–AF15
 *  - Atomic set/reset via BSRR
 *  - Toggle via ODR XOR
 *  - Error handling for invalid port / pin / AF arguments
 *
 * Peripheral clock is enabled automatically inside gpio_init().
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>   /* for NULL */

/* ====================================================================
 * Enumerations
 * ==================================================================== */

/** @brief GPIO pin operating mode (MODER register, 2 bits per pin). */
typedef enum
{
    GPIO_MODE_INPUT  = 0x00U, /**< Digital input (default after reset) */
    GPIO_MODE_OUTPUT = 0x01U, /**< General-purpose output              */
    GPIO_MODE_AF     = 0x02U, /**< Alternate function                  */
    GPIO_MODE_ANALOG = 0x03U  /**< Analog / ADC mode                   */
} GPIO_Mode_t;

/** @brief GPIO output type (OTYPER register, 1 bit per pin). */
typedef enum
{
    GPIO_OTYPE_PP = 0x00U, /**< Push-pull (default) */
    GPIO_OTYPE_OD = 0x01U  /**< Open-drain          */
} GPIO_OType_t;

/** @brief GPIO output speed (OSPEEDR register, 2 bits per pin). */
typedef enum
{
    GPIO_SPEED_LOW    = 0x00U, /**< ~2  MHz  */
    GPIO_SPEED_MEDIUM = 0x01U, /**< ~25 MHz  */
    GPIO_SPEED_HIGH   = 0x02U, /**< ~50 MHz  */
    GPIO_SPEED_VHIGH  = 0x03U  /**< ~100 MHz */
} GPIO_Speed_t;

/** @brief GPIO pull-up / pull-down (PUPDR register, 2 bits per pin). */
typedef enum
{
    GPIO_PUPD_NONE     = 0x00U, /**< No pull (default) */
    GPIO_PUPD_PULLUP   = 0x01U, /**< Internal pull-up  */
    GPIO_PUPD_PULLDOWN = 0x02U  /**< Internal pull-down */
} GPIO_PUPD_t;

/** @brief Alternate function selection (AFRx register, 4 bits per pin). */
typedef enum
{
    GPIO_AF0  = 0U,  GPIO_AF1  = 1U,  GPIO_AF2  = 2U,  GPIO_AF3  = 3U,
    GPIO_AF4  = 4U,  GPIO_AF5  = 5U,  GPIO_AF6  = 6U,  GPIO_AF7  = 7U,
    GPIO_AF8  = 8U,  GPIO_AF9  = 9U,  GPIO_AF10 = 10U, GPIO_AF11 = 11U,
    GPIO_AF12 = 12U, GPIO_AF13 = 13U, GPIO_AF14 = 14U, GPIO_AF15 = 15U
} GPIO_AF_t;

/** @brief Driver return status. */
typedef enum
{
    GPIO_OK    = 0U, /**< Operation successful */
    GPIO_ERROR = 1U  /**< Invalid argument      */
} GPIO_Status_t;

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief Configure a GPIO pin mode; enables the peripheral clock.
 * @param port  GPIOA … GPIOE pointer.
 * @param pin   Pin number 0–15.
 * @param mode  GPIO_MODE_INPUT / OUTPUT / AF / ANALOG.
 * @return GPIO_OK or GPIO_ERROR.
 */
GPIO_Status_t gpio_init(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode);

/** @brief Set push-pull or open-drain output type. */
GPIO_Status_t gpio_set_output_type(GPIO_TypeDef *port, uint8_t pin,
                                   GPIO_OType_t otype);

/** @brief Set output speed (slew rate). */
GPIO_Status_t gpio_set_speed(GPIO_TypeDef *port, uint8_t pin,
                              GPIO_Speed_t speed);

/** @brief Configure internal pull-up / pull-down resistor. */
GPIO_Status_t gpio_set_pupd(GPIO_TypeDef *port, uint8_t pin,
                             GPIO_PUPD_t pupd);

/**
 * @brief Configure alternate function for a pin (AFRx register).
 *        Pin must already be in GPIO_MODE_AF via gpio_init().
 * @param af    AF0–AF15.
 * @return GPIO_OK or GPIO_ERROR.
 */
GPIO_Status_t gpio_set_af(GPIO_TypeDef *port, uint8_t pin, GPIO_AF_t af);

/**
 * @brief Write a logic level to an output pin (atomic via BSRR).
 * @param value  1 = set high, 0 = set low.
 */
void    gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value);

/** @brief Toggle the current output level (XOR on ODR). */
void    gpio_toggle(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief Read the current logic level from IDR.
 * @return 1 if high, 0 if low.
 */
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

#endif /* GPIO_DRIVER_H */
