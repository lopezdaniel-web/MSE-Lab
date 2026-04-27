/**
 * @file    led.h
 * @brief   On-board LED driver for NUCLEO-F411RE.
 *
 * Hardware mapping (fixed):
 *   Pin  : PA5  (LD2, active-high green LED)
 *   Port : GPIOA
 *   Mode : Push-pull output, low speed
 */

#ifndef LED_H
#define LED_H

/**
 * @brief  Configure PA5 as a push-pull output and drive it LOW.
 *         Enables GPIOA peripheral clock via gpio_init().
 */
void led_init(void);

/** @brief  Drive PA5 HIGH – LED on.  */
void led_on(void);

/** @brief  Drive PA5 LOW  – LED off. */
void led_off(void);

/** @brief  Toggle the current state of PA5. */
void led_toggle(void);

#endif /* LED_H */
