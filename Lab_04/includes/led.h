/**
 * @file    led.h
 * @brief   On-board LED driver – PA5 (LD2) on NUCLEO-F411RE.
 */

#ifndef LED_H
#define LED_H

void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif /* LED_H */