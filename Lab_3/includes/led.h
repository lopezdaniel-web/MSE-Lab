/**
 * @file    led.h
 * @brief   On-board LED module – LD2 on PA5 (NUCLEO-F411RE).
 *
 * Hardware mapping (fixed):
 *   LED     : LD2 (green user LED on the NUCLEO-F411RE)
 *   Pin     : PA5  →  GPIO_MODE_OUTPUT, push-pull, low speed, no pull
 *   Active  : HIGH (writing 1 turns the LED on)
 *
 * This module wraps the GPIO driver to provide a minimal four-function
 * interface for the heartbeat LED used across the lab assignments.
 *
 * Usage:
 *   led_init();          // once at startup
 *   led_on();            // turn LED on
 *   led_off();           // turn LED off
 *   led_toggle();        // invert current state
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>

/**
 * @brief  Initialise PA5 as a push-pull output for LD2.
 *
 *         - Enables GPIOA AHB1 clock (handled inside gpio_init()).
 *         - Sets PA5 to GPIO_MODE_OUTPUT, push-pull, low speed, no pull.
 *         - Drives the pin LOW so the LED starts off.
 *
 *         Must be called once at startup before any other led_* function.
 */
void led_init(void);

/** @brief  Turn LD2 on  (drives PA5 HIGH). */
void led_on(void);

/** @brief  Turn LD2 off (drives PA5 LOW).  */
void led_off(void);

/** @brief  Toggle LD2 current state.       */
void led_toggle(void);

#endif /* LED_H */
