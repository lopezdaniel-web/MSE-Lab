/**
 ******************************************************************************
 * @file    button.c
 * @brief   Debounced reading of B1 (PC13) on Nucleo-F411RE.
 *
 * Hardware
 * ────────
 *   B1 → PC13, active-LOW.
 *   IDR = 0 when the button is PRESSED.
 *   IDR = 1 when the button is RELEASED.
 *
 * Debounce
 * ────────
 *   8-bit shift register (history):
 *   - Filled with raw reading on each call.
 *   - 0x00 (8 consecutive samples in LOW)  → BUTTON_PRESSED
 *   - 0xFF (8 consecutive samples in HIGH) → BUTTON_RELEASED
 *   - Any mixed pattern                     → unchanged state (noise)
 *
 * The function uses gpio_driver to initialize the pin; the raw reading
 * accesses GPIOC->IDR directly via gpio_read().
 ******************************************************************************
 */

#include "includes/button.h"
#include "includes/gpio_driver.h"   /* gpio_init, gpio_read; incluye stm32f4xx.h */

#define BUTTON_PIN  13U    /* PC13 = B1 */

void button_init(void)
{
    /* PC13: digital input*/
    gpio_init(GPIOC, BUTTON_PIN, GPIO_MODE_INPUT, GPIO_NOPULL);
}

uint8_t button_get_state(void)
{
    /* History: 0xFF = released (all samples in HIGH) */
    static uint8_t history = 0xFFU;
    static uint8_t state   = BUTTON_RELEASED;

    /* Shift 1 bit left and insert current reading into LSB.
     * gpio_read returns 0 when button is pressed (active-low). */
    history = (uint8_t)((history << 1U) | gpio_read(GPIOC, BUTTON_PIN));

    if      (history == 0x00U) { state = BUTTON_PRESSED;  }
    else if (history == 0xFFU) { state = BUTTON_RELEASED; }
    /* If history is mixed, state doesn't change → immune to bounces */

    return state;
}
