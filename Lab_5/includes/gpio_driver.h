/**
 * @file    gpio_driver.h
 * @brief   Low-level GPIO driver for STM32F4xx (CMSIS / bare-metal).
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>

typedef enum { GPIO_MODE_INPUT=0, GPIO_MODE_OUTPUT=1, GPIO_MODE_AF=2, GPIO_MODE_ANALOG=3 }      GPIO_Mode_t;
typedef enum { GPIO_OTYPE_PP=0, GPIO_OTYPE_OD=1 }                                               GPIO_OType_t;
typedef enum { GPIO_SPEED_LOW=0, GPIO_SPEED_MEDIUM=1, GPIO_SPEED_HIGH=2, GPIO_SPEED_VHIGH=3 }   GPIO_Speed_t;
typedef enum { GPIO_PUPD_NONE=0, GPIO_PUPD_PULLUP=1, GPIO_PUPD_PULLDOWN=2 }                     GPIO_PUPD_t;
typedef enum { GPIO_AF0=0,GPIO_AF1=1,GPIO_AF2=2,GPIO_AF3=3,GPIO_AF4=4,GPIO_AF5=5,
               GPIO_AF6=6,GPIO_AF7=7,GPIO_AF8=8,GPIO_AF9=9,GPIO_AF10=10,GPIO_AF11=11,
               GPIO_AF12=12,GPIO_AF13=13,GPIO_AF14=14,GPIO_AF15=15 }                            GPIO_AF_t;
typedef enum { GPIO_OK=0, GPIO_ERROR=1 }                                                        GPIO_Status_t;

GPIO_Status_t gpio_init(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode);
GPIO_Status_t gpio_set_output_type(GPIO_TypeDef *port, uint8_t pin, GPIO_OType_t otype);
GPIO_Status_t gpio_set_speed(GPIO_TypeDef *port, uint8_t pin, GPIO_Speed_t speed);
GPIO_Status_t gpio_set_pupd(GPIO_TypeDef *port, uint8_t pin, GPIO_PUPD_t pupd);
GPIO_Status_t gpio_set_af(GPIO_TypeDef *port, uint8_t pin, GPIO_AF_t af);
void          gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value);
void          gpio_toggle(GPIO_TypeDef *port, uint8_t pin);
uint8_t       gpio_read(GPIO_TypeDef *port, uint8_t pin);

#endif /* GPIO_DRIVER_H */