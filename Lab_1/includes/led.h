/**
 * @file  led.h
 * @brief Simple HAL for LD2 (PA5) on the Nucleo-F411RE.
 *
 * The LED is active-high and physically connected to PA5.
 * Clock and mode configuration is done inside led_init().
 */

#ifndef LED_H          /* uppercase guard – conventional in C                 */
#define LED_H

void led_init   (void);   /**< Enable GPIOA clock, configure PA5 as output.   */
void led_on     (void);   /**< Drive PA5 HIGH (LED on).                        */
void led_off    (void);   /**< Drive PA5 LOW  (LED off).                       */
void led_toggle (void);   /**< Flip PA5 state.                                 */

#endif /* LED_H */
