/**
 * @file    tim_driver.h
 * @brief   Low-level TIM driver for STM32F4xx (CMSIS / bare-metal).
 *
 * Supports timer init, counter control, UIF polling,
 * PWM output mode config, and channel enable/disable.
 *
 * NOTE: No OC preload (OC1PE) is used — CCR writes take effect
 *       immediately, consistent with the verified working implementation.
 */

#ifndef TIM_DRIVER_H
#define TIM_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>

#define TIM_CLOCK_HZ    (16000000U)   /* HSI 16 MHz, APBx prescaler = 1 */

typedef enum { TIM_OK=0, TIM_ERROR=1 }                  TIM_Status_t;
typedef enum { TIM_CHANNEL_1=1, TIM_CHANNEL_2=2,
               TIM_CHANNEL_3=3, TIM_CHANNEL_4=4 }       TIM_Channel_t;
typedef enum { TIM_PWM_MODE_1=6, TIM_PWM_MODE_2=7 }     TIM_PWM_Mode_t;

/* Core timer control */
TIM_Status_t tim_init(TIM_TypeDef *tim, uint16_t psc, uint32_t arr);
void         tim_enable(TIM_TypeDef *tim);
void         tim_disable(TIM_TypeDef *tim);
void         tim_reset_counter(TIM_TypeDef *tim);
uint32_t     tim_get_counter(TIM_TypeDef *tim);

/* Status flags */
uint8_t      tim_get_update_flag(TIM_TypeDef *tim);
void         tim_clear_update_flag(TIM_TypeDef *tim);

/* Frequency configuration (calculates PSC/ARR automatically) */
TIM_Status_t tim_set_frequency(TIM_TypeDef *tim, uint32_t freq_hz);

/* PWM configuration — no OC preload, CCR takes effect immediately */
TIM_Status_t tim_pwm_config(TIM_TypeDef *tim, TIM_Channel_t channel,
                             TIM_PWM_Mode_t mode);
TIM_Status_t tim_set_ccr(TIM_TypeDef *tim, TIM_Channel_t channel,
                          uint32_t value);
uint32_t     tim_get_arr(TIM_TypeDef *tim);

/* Channel output enable/disable */
TIM_Status_t tim_channel_enable(TIM_TypeDef *tim, TIM_Channel_t channel);
TIM_Status_t tim_channel_disable(TIM_TypeDef *tim, TIM_Channel_t channel);

#endif /* TIM_DRIVER_H */