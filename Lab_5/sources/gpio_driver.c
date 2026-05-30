/**
 * @file    gpio_driver.c
 * @brief   Low-level GPIO driver implementation (STM32F4xx, CMSIS).
 */

#include "gpio_driver.h"

/* ── Private: enable AHB1 clock for a port ───────────────────────── */
static void gpio_enable_clock(GPIO_TypeDef *port)
{
    if      (port == GPIOA) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; }
    else if (port == GPIOB) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; }
    else if (port == GPIOC) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; }
    else if (port == GPIOD) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; }
    else if (port == GPIOE) { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; }
}

/* ── gpio_init ───────────────────────────────────────────────────── */
GPIO_Status_t gpio_init(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode)
{
    if ((port == NULL) || (pin > 15U)) { return GPIO_ERROR; }

    gpio_enable_clock(port);
    port->MODER &= ~(0x3U << (pin * 2U));
    port->MODER |=  ((uint32_t)mode << (pin * 2U));
    return GPIO_OK;
}

/* ── gpio_set_output_type ────────────────────────────────────────── */
GPIO_Status_t gpio_set_output_type(GPIO_TypeDef *port, uint8_t pin, GPIO_OType_t otype)
{
    if ((port == NULL) || (pin > 15U)) { return GPIO_ERROR; }
    port->OTYPER &= ~(0x1U << pin);
    port->OTYPER |=  ((uint32_t)otype << pin);
    return GPIO_OK;
}

/* ── gpio_set_speed ──────────────────────────────────────────────── */
GPIO_Status_t gpio_set_speed(GPIO_TypeDef *port, uint8_t pin, GPIO_Speed_t speed)
{
    if ((port == NULL) || (pin > 15U)) { return GPIO_ERROR; }
    port->OSPEEDR &= ~(0x3U << (pin * 2U));
    port->OSPEEDR |=  ((uint32_t)speed << (pin * 2U));
    return GPIO_OK;
}

/* ── gpio_set_pupd ───────────────────────────────────────────────── */
GPIO_Status_t gpio_set_pupd(GPIO_TypeDef *port, uint8_t pin, GPIO_PUPD_t pupd)
{
    if ((port == NULL) || (pin > 15U)) { return GPIO_ERROR; }
    port->PUPDR &= ~(0x3U << (pin * 2U));
    port->PUPDR |=  ((uint32_t)pupd << (pin * 2U));
    return GPIO_OK;
}

/* ── gpio_set_af ─────────────────────────────────────────────────── */
GPIO_Status_t gpio_set_af(GPIO_TypeDef *port, uint8_t pin, GPIO_AF_t af)
{
    if ((port == NULL) || (pin > 15U) || ((uint8_t)af > 15U)) { return GPIO_ERROR; }

    if (pin < 8U)
    {
        port->AFR[0] &= ~(0xFU << (pin * 4U));
        port->AFR[0] |=  ((uint32_t)af << (pin * 4U));
    }
    else
    {
        uint8_t offset = pin - 8U;
        port->AFR[1] &= ~(0xFU << (offset * 4U));
        port->AFR[1] |=  ((uint32_t)af << (offset * 4U));
    }
    return GPIO_OK;
}

/* ── gpio_write (atomic via BSRR) ───────────────────────────────── */
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value)
{
    if (value) { port->BSRR =  (1U << pin);        }
    else       { port->BSRR =  (1U << (pin + 16U)); }
}

/* ── gpio_toggle ─────────────────────────────────────────────────── */
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin)
{
    port->ODR ^= (1U << pin);
}

/* ── gpio_read ───────────────────────────────────────────────────── */
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (uint8_t)((port->IDR >> pin) & 0x1U);
}