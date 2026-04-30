/**
 * @file    gpio_driver.c
 * @brief   Low-level GPIO driver implementation (STM32F4xx, CMSIS).
 *
 * Register map used:
 *   MODER    [2n+1 : 2n]  – pin mode (2 bits per pin)
 *   OTYPER   [n]          – output type (1 bit per pin)
 *   OSPEEDR  [2n+1 : 2n]  – output speed (2 bits per pin)
 *   PUPDR    [2n+1 : 2n]  – pull-up/pull-down (2 bits per pin)
 *   AFR[0]   [4n+3 : 4n]  – alternate function pins 0–7  (AFRL)
 *   AFR[1]   [4n+3 : 4n]  – alternate function pins 8–15 (AFRH)
 *   BSRR     [n]    set   – atomic set bit n HIGH
 *   BSRR     [n+16] reset – atomic set bit n LOW
 *   ODR      [n]          – output data (XOR for toggle)
 *   IDR      [n]          – input data (read-only)
 */
#include "gpio_driver.h"

/* ====================================================================
 * Private – Peripheral Clock Enable
 * ==================================================================== */

/**
 * @brief Enable the AHB1 peripheral clock for the given GPIO port.
 *        Called automatically at the start of gpio_init().
 */
static void gpio_enable_clock(GPIO_TypeDef *port)
{
    if      (port == GPIOA) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; }
    else if (port == GPIOB) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; }
    else if (port == GPIOC) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; }
    else if (port == GPIOD) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; }
    else if (port == GPIOE) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; }
    /* Extend for GPIOH if needed on this device */
}

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief  Configure pin mode (MODER[2n+1:2n]).
 *         Enables the GPIO peripheral clock before modifying registers.
 *
 * Read-Modify-Write pattern:
 *   Clear: MODER &= ~(0x3 << pin*2)
 *   Set  : MODER |=  (mode << pin*2)
 */
GPIO_Status_t gpio_init(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode)
{
    if ((port == NULL) || (pin > 15U))
    {
        return GPIO_ERROR;
    }

    gpio_enable_clock(port);

    port->MODER &= ~(0x3U  << (pin * 2U));
    port->MODER |=  ((uint32_t)mode << (pin * 2U));

    return GPIO_OK;
}

/**
 * @brief  Set output type register (OTYPER[n]).
 *         0 = push-pull, 1 = open-drain.
 */
GPIO_Status_t gpio_set_output_type(GPIO_TypeDef *port, uint8_t pin,
                                   GPIO_OType_t otype)
{
    if ((port == NULL) || (pin > 15U))
    {
        return GPIO_ERROR;
    }

    port->OTYPER &= ~(0x1U  << pin);
    port->OTYPER |=  ((uint32_t)otype << pin);

    return GPIO_OK;
}

/**
 * @brief  Set output speed (OSPEEDR[2n+1:2n]).
 */
GPIO_Status_t gpio_set_speed(GPIO_TypeDef *port, uint8_t pin,
                              GPIO_Speed_t speed)
{
    if ((port == NULL) || (pin > 15U))
    {
        return GPIO_ERROR;
    }

    port->OSPEEDR &= ~(0x3U  << (pin * 2U));
    port->OSPEEDR |=  ((uint32_t)speed << (pin * 2U));

    return GPIO_OK;
}

/**
 * @brief  Configure pull-up / pull-down (PUPDR[2n+1:2n]).
 */
GPIO_Status_t gpio_set_pupd(GPIO_TypeDef *port, uint8_t pin,
                             GPIO_PUPD_t pupd)
{
    if ((port == NULL) || (pin > 15U))
    {
        return GPIO_ERROR;
    }

    port->PUPDR &= ~(0x3U  << (pin * 2U));
    port->PUPDR |=  ((uint32_t)pupd << (pin * 2U));

    return GPIO_OK;
}

/**
 * @brief  Set the alternate function for a pin (AFR[0] or AFR[1]).
 *
 *  Pins 0–7  → AFR[0] (AFRL): field at bits [4*(pin)   + 3 : 4*(pin)]
 *  Pins 8–15 → AFR[1] (AFRH): field at bits [4*(pin-8) + 3 : 4*(pin-8)]
 *
 * @note   The pin must have been configured in GPIO_MODE_AF via gpio_init().
 */
GPIO_Status_t gpio_set_af(GPIO_TypeDef *port, uint8_t pin, GPIO_AF_t af)
{
    if ((port == NULL) || (pin > 15U) || ((uint8_t)af > 15U))
    {
        return GPIO_ERROR;
    }

    if (pin < 8U)
    {
        /* AFRL – pins 0–7 */
        port->AFR[0] &= ~(0xFU << (pin * 4U));
        port->AFR[0] |=  ((uint32_t)af << (pin * 4U));
    }
    else
    {
        /* AFRH – pins 8–15 */
        uint8_t offset = pin - 8U;
        port->AFR[1] &= ~(0xFU << (offset * 4U));
        port->AFR[1] |=  ((uint32_t)af << (offset * 4U));
    }

    return GPIO_OK;
}

/**
 * @brief  Atomically drive an output pin HIGH or LOW via BSRR.
 *
 *  BSRR[15:0]  (BSx) – writing 1 sets   ODR bit n HIGH  (no read needed)
 *  BSRR[31:16] (BRx) – writing 1 resets ODR bit n LOW   (no read needed)
 */
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value)
{
    if (value)
    {
        port->BSRR = (1U << pin);            /* Set   */
    }
    else
    {
        port->BSRR = (1U << (pin + 16U));    /* Reset */
    }
}

/**
 * @brief  Toggle the output level of a pin (XOR on ODR).
 *         Note: not atomic – use only when no concurrent access exists.
 */
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin)
{
    port->ODR ^= (1U << pin);
}

/**
 * @brief  Read the current logic level from IDR.
 * @return 1 if the pin is HIGH, 0 if LOW.
 */
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (uint8_t)((port->IDR >> pin) & 0x1U);
}
