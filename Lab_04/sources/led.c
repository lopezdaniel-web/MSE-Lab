/**
 * @file    led.c
 * @brief   On-board LED LD2 – PA5 on NUCLEO-F411RE.
 */

#include "led.h"
#include "gpio_driver.h"

#define LED_PORT    GPIOA
#define LED_PIN     (5U)

void led_init(void)
{
    gpio_init(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type(LED_PORT, LED_PIN, GPIO_OTYPE_PP);
    gpio_set_speed(LED_PORT, LED_PIN, GPIO_SPEED_LOW);
    gpio_set_pupd(LED_PORT, LED_PIN, GPIO_PUPD_NONE);
    gpio_write(LED_PORT, LED_PIN, 0U);
}

void led_on(void)     { gpio_write(LED_PORT, LED_PIN, 1U); }
void led_off(void)    { gpio_write(LED_PORT, LED_PIN, 0U); }
void led_toggle(void) { gpio_toggle(LED_PORT, LED_PIN);    }