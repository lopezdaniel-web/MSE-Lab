/**
 ******************************************************************************
 * @file    led.c
 * @brief   Control of LD2 (PA5) on Nucleo-F411RE using CMSIS registers.
 *
 * Direct hardware access (no intermediate abstraction):
 *   RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN;   enables GPIOA clock
 *   GPIOA->MODER  → configures PA5 as output (MODER=01)
 *   GPIOA->BSRR   → atomic set/reset (preferred over ODR for set/clear)
 *   GPIOA->ODR    → toggle via XOR
 ******************************************************************************
 */

#include "led.h"
#include "stm32f4xx.h"   /* GPIOA, RCC, RCC_AHB1ENR_GPIOAEN – provistos por CMSIS */

#define LED_PIN     5U   /* PA5 = LD2 en la Nucleo-F411RE */

void led_init(void)
{
    /* Enable AHB1 bus clock for GPIOA */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA5 → general purpose output mode (MODER[11:10] = 01b)
     * Step 1: clear the 2 bits of the field to avoid glitch when writing */
    GPIOA->MODER &= ~(3UL << (LED_PIN * 2U));
    /* Step 2: write 01b (output) */
    GPIOA->MODER |=  (1UL << (LED_PIN * 2U));
}

void led_on(void)
{
    /* BSRR bits [15:0] → set (BS5 = bit 5) */
    GPIOA->BSRR = (1UL << LED_PIN);
}

void led_off(void)
{
    /* BSRR bits [31:16] → reset (BR5 = bit 21) */
    GPIOA->BSRR = (1UL << (LED_PIN + 16U));
}

void led_toggle(void)
{
    /* XOR: toggles only the LED bit */
    GPIOA->ODR ^= (1UL << LED_PIN);
}
