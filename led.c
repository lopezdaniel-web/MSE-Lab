#include "led.h"

void led_init() {
    GPIOA_MODER &= ~(3UL << LED_MODER);
    GPIOA_MODER |=  (1UL << LED_MODER);
}

void encender() {
    GPIOA_ODR |=  (1UL << LED_PIN);
}

void apagar() {
    GPIOA_ODR &= ~(1UL << LED_PIN);
}