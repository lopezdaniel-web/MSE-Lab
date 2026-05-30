/**
 * @file    serial.h
 * @brief   High-level serial communication module for USART2.
 *
 * Wraps the UART driver and uses utils_snprintf() to provide a
 * printf-style API for sending formatted strings to a serial terminal.
 *
 * Supported format specifiers (inherited from utils_snprintf):
 *   %d  signed decimal integer
 *   %u  unsigned decimal integer
 *   %x  hexadecimal integer
 *   %s  string
 *   %c  character
 *   %%  literal percent sign
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

/** @brief Maximum length of a formatted message (bytes including '\0'). */
#define SERIAL_BUF_SIZE  (128U)

/**
 * @brief  Initialise USART2 at 115200 baud.
 *         Calls uart_init() internally; must be called before serial_printf().
 */
void serial_init(void);

/**
 * @brief  Format and transmit a string over USART2.
 *
 *  1. Formats the message into a local buffer using utils_snprintf().
 *  2. Transmits each character in the buffer with uart_write().
 *
 * @param  format  printf-style format string.
 * @param  ...     Variable arguments matching the format specifiers.
 */
void serial_printf(const char *format, ...);

#endif /* SERIAL_H */