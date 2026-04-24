#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

// Base addresses
#define GPIOA_BASE         (0x40020000UL)
#define GPIOC_BASE         (0x40020800UL)
#define RCC_BASE           (0x40023800UL)

// Registers GPIOA (LED)
#define GPIOA_MODER        (*(volatile unsigned int *)(GPIOA_BASE + 0x00UL))
#define GPIOA_ODR          (*(volatile unsigned int *)(GPIOA_BASE + 0x14UL))

// Registers GPIOC (BUTTON)
#define GPIOC_MODER        (*(volatile unsigned int *)(GPIOC_BASE + 0x00UL))
#define GPIOC_IDR          (*(volatile unsigned int *)(GPIOC_BASE + 0x10UL))

// RCC
#define RCC_AHB1ENR        (*(volatile unsigned int *)(RCC_BASE   + 0x30UL))

// LED pin PA5
#define LED_PIN            5
#define LED_MODER          (2 * LED_PIN)

// BUTTON pin PC13
#define BTN_PIN            13
#define BTN_MODER          (2 * BTN_PIN)

void gpio_init();

#endif