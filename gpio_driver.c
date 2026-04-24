#include "gpio_driver.h"

void gpio_init() {
    RCC_AHB1ENR |= (1UL << 0);   // reloj GPIOA
    RCC_AHB1ENR |= (1UL << 2);   // reloj GPIOC
}

