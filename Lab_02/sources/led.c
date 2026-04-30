/**
 * @file    led.c
 * @brief   On-board LED (LD2) driver – PA5 on NUCLEO-F411RE.
 *
 * LD2 is connected between PA5 and GND through a 510 Ω resistor.
 * Logic HIGH → LED on, Logic LOW → LED off.
 */

#include "led.h"
#include "gpio_driver.h"

/* ====================================================================
 * Private Constants
 * ==================================================================== */

#define LED_PORT    GPIOA   /**< LED GPIO port                  */
#define LED_PIN     (5U)    /**< PA5 = LD2 on NUCLEO-F411RE    */

/* ====================================================================
 * Public API
 * ==================================================================== */

/**
 * @brief  Initialise PA5 as a push-pull output and drive it LOW.
 *         gpio_init() enables the GPIOA AHB1 clock automatically.
 */
void led_init(void)
{
    gpio_init(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type(LED_PORT, LED_PIN, GPIO_OTYPE_PP);
    gpio_set_speed(LED_PORT, LED_PIN, GPIO_SPEED_LOW);
    gpio_set_pupd(LED_PORT, LED_PIN, GPIO_PUPD_NONE);
    gpio_write(LED_PORT, LED_PIN, 0U);   /* LED off at startup */
}

/** @brief  Drive PA5 HIGH → LD2 illuminates. */
void led_on(void)
{
    gpio_write(LED_PORT, LED_PIN, 1U);
}

/** @brief  Drive PA5 LOW → LD2 off. */
void led_off(void)
{
    gpio_write(LED_PORT, LED_PIN, 0U);
}

/** @brief  Toggle PA5 state. */
void led_toggle(void)
{
    gpio_toggle(LED_PORT, LED_PIN);
}
