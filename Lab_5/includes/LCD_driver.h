/**
 * @file    lcd_driver.h
 * @brief   HD44780 - LCD driver – 4-bit parallel mode.
 *
 * Wiring (6 GPIO pins total – no SPI, no I2C):
 *
 *   STM32 pin   LCD pin   Signal
 *   ─────────   ───────   ──────
 *   PB5         4         RS   (Register Select: 0=command, 1=data)
 *   PB6         6         E    (Enable – latches data on falling edge)
 *   PB7         11        D4   ┐
 *   PB8         12        D5   │ 4-bit data bus
 *   PB9         13        D6   │ (D0–D3 of the LCD left unconnected)
 *   PB10        14        D7   ┘
 *
 *   LCD pin 1  → GND
 *   LCD pin 2  → +5 V
 *   LCD pin 3  → potentiometer wiper  (contrast, 0–5 V)
 *   LCD pin 5  → GND                  (RW tied low → write-only)
 *   LCD pin 15 → +5 V (via 33 Ω)      (backlight +)
 *   LCD pin 16 → GND                  (backlight −)
 *
 * Configuration:
 *   Interface   : 4-bit
 *   Lines       : 2
 *   Font        : 5×8 dots
 *   Cursor      : OFF
 *   Blink       : OFF
 *
 * Dependencies:
 *   gpio_driver  (gpio_init, gpio_write, gpio_set_*)
 *   timer        (timer_delay_ms)
 */

#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
 
/* ── Pin / port definitions ──────────────────────────────────────── */
#define LCD_PORT        GPIOB
 
#define LCD_PIN_RS      5U      /**< PB5  – Register Select.              */
#define LCD_PIN_E       6U      /**< PB6  – Enable.                       */
#define LCD_PIN_D4      7U      /**< PB7  – Data bit 4.                   */
#define LCD_PIN_D5      12U     /**< PB12 – Data bit 5 (moved from PB8).  */
#define LCD_PIN_D6      13U     /**< PB13 – Data bit 6 (moved from PB9).  */
#define LCD_PIN_D7      10U     /**< PB10 – Data bit 7.                   */
 
/* ── HD44780 command codes ───────────────────────────────────────── */
#define LCD_CMD_CLEAR           0x01U
#define LCD_CMD_HOME            0x02U
#define LCD_CMD_ENTRY_MODE      0x06U
#define LCD_CMD_DISPLAY_OFF     0x08U
#define LCD_CMD_DISPLAY_ON      0x0CU
#define LCD_CMD_FUNCTION_SET    0x28U
 
/* ── DDRAM row base addresses (16×2 LCD) ────────────────────────── */
#define LCD_ROW0_ADDR           0x00U
#define LCD_ROW1_ADDR           0x40U
 
/* ── Display geometry ────────────────────────────────────────────── */
#define LCD_COLS                16U
#define LCD_ROWS                2U
 
/* ── Return type ─────────────────────────────────────────────────── */
typedef enum
{
    LCD_OK    = 0,
    LCD_ERROR = 1
} LCD_Status_t;
 
/* ── Public API ──────────────────────────────────────────────────── */
 
/** @brief Initialize GPIO pins and run the HD44780 power-on sequence. */
void lcd_init(void);
 
/** @brief Overwrite current row with 16 spaces (software clear). */
void lcd_clear(void);
 
/** @brief Move cursor to (row, col). row: 0–1, col: 0–15. */
LCD_Status_t lcd_set_cursor(uint8_t row, uint8_t col);
 
/** @brief Write one ASCII character at the current position. */
void lcd_print_char(char ch);
 
/** @brief Write a null-terminated string. Returns LCD_ERROR if str is NULL. */
LCD_Status_t lcd_print_string(const char *str);
 
/* ── Future implementations ──────────────────────────────────────── */
// void lcd_print_int(int32_t value);   /* use utils_snprintf + lcd_print_string instead */
 
#endif /* LCD_DRIVER_H */