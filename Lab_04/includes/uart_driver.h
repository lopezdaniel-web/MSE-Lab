/**
 * @file    uart_driver.h
 * @brief   Low-level UART driver for USART2 (CMSIS / bare-metal).
 *
 * Hardware mapping (fixed – NUCLEO-F411RE ST-Link virtual COM):
 *   TX : PA2  →  AF7 (USART2_TX)
 *   RX : PA3  →  AF7 (USART2_RX)  [configured but not used for TX-only labs]
 *
 * Clock:
 *   USART2 is on APB1  →  PCLK1 = 16 MHz (HSI, PPRE1 = /1)
 *
 * BRR calculation (OVER8 = 0, 16x oversampling):
 *   USARTDIV = PCLK1 / (16 × baud)
 *   At 115200: USARTDIV = 16 000 000 / 1 843 200 = 8.680
 *   DIV_Mantissa = 8  →  bits [15:4]
 *   DIV_Fraction = round(0.680 × 16) = 11  →  bits [3:0]
 *   BRR = (8 << 4) | 11 = 139
 */

#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>

/* ── Baud rate constant ──────────────────────────────────────────── */
#define UART_BAUD_115200    (139U)   /* BRR value for 115200 at 16 MHz */
#define UART_BAUD_9600      (1667U)  /* BRR value for   9600 at 16 MHz */

/* ── Status ──────────────────────────────────────────────────────── */
typedef enum { UART_OK = 0U, UART_ERR = 1U } uart_status_t;

/* ── Public API ──────────────────────────────────────────────────── */

/**
 * FR-1 Enable USART2 peripheral clock (APB1).
 * FR-2 Configure baud rate via BRR register.
 * FR-3 Enable transmitter (CR1.TE) and peripheral (CR1.UE).
 *      Also configures PA2 as AF7 (TX) and PA3 as AF7 (RX).
 *
 * @param brr  Pre-calculated BRR value (use UART_BAUD_115200 etc.)
 */
uart_status_t uart_init(uint32_t brr);

/**
 * FR-4 Poll SR.TXE until the transmit data register is empty.
 * FR-5 Write the byte to DR for transmission.
 *
 * @param byte  8-bit character to transmit.
 */
void uart_write(uint8_t byte);

#endif /* UART_DRIVER_H */