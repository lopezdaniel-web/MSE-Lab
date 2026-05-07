/**
 * @file    led.c
 * @brief   On-board LED module implementation – LD2 on PA5.
 *
 * ── Hardware connection (NUCLEO-F411RE) ──────────────────────────────
 *
 *   LD2 (green user LED) is connected to PA5 through a series resistor.
 *   The LED is active-HIGH: PA5 = 1 → LED on, PA5 = 0 → LED off.
 *   Same line is shared with the Arduino-style D13 header on CN5 pin 6.
 *
 * ── Module dependencies ──────────────────────────────────────────────
 *
 *   gpio_driver – all register-level GPIO operations
 */

#include "led.h"
#include "gpio_driver.h"
#include "stm32f411xe.h"

/* ====================================================================
 * Private Constants
 * ==================================================================== */

#define LED_GPIO_PORT   GPIOA   /**< GPIO port for LD2 */
#define LED_GPIO_PIN    (5U)    /**< PA5 = LD2         */

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief  Initialise PA5 as a push-pull output, LED off.
 *
 * Steps:
 *  1. gpio_init() with GPIO_MODE_OUTPUT (clock enabled internally).
 *  2. gpio_set_output_type() → push-pull.
 *  3. gpio_set_speed() → low speed (sufficient for an LED).
 *  4. gpio_set_pupd() → no pull (the LED + resistor define the line state).
 *  5. gpio_write() → 0, so LD2 starts off.
 */
void led_init(void)
{
    gpio_init(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_OTYPE_PP);
    gpio_set_speed(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_SPEED_LOW);
    gpio_set_pupd(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PUPD_NONE);

    gpio_write(LED_GPIO_PORT, LED_GPIO_PIN, 0U);
}

/**
 * @brief  Drive PA5 HIGH → LD2 on.
 */
void led_on(void)
{
    gpio_write(LED_GPIO_PORT, LED_GPIO_PIN, 1U);
}

/**
 * @brief  Drive PA5 LOW → LD2 off.
 */
void led_off(void)
{
    gpio_write(LED_GPIO_PORT, LED_GPIO_PIN, 0U);
}

/**
 * @brief  Invert current PA5 state via gpio_toggle().
 *         Used as the heartbeat indicator in the super-loop.
 */
void led_toggle(void)
{
    gpio_toggle(LED_GPIO_PORT, LED_GPIO_PIN);
}
