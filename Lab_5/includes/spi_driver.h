/**
 * @file    spi_driver.h
 * @brief   Low-level SPI driver for STM32F4xx – SPI1, bare-metal / CMSIS.
 *
 * Hardware mapping (Nucleo-F411RE ↔ LCD module):
 *   PA5  → SCK   (AF5)
 *   PA6  → MISO  (AF5)
 *   PA7  → MOSI  (AF5)
 *   PA8  → 
 *   PA9  → CS    (GPIO output – software controlled)
 *
 * Default configuration:
 *   Mode   : Master
 *   Clock  : SPI Mode 3 (CPOL=1, CPHA=1)
 *   Baud   : fPCLK/32  ≈ 3.125 MHz  (APB2 @ 100 MHz)
 *   Frame  : 8-bit, MSB first
 *   NSS    : Software (SSM=1, SSI=1)
 *
 * References:
 *   - STM32F411RE Datasheet
 *   - RM0383 Reference Manual – Section 20 (SPI)
 *   - SRS-SPI_Driver v1.0 (FR-1 … FR-8, NFR-1 … NFR-3)
 */

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>

/* ── Return type ─────────────────────────────────────────────────── */
typedef enum
{
    SPI_OK    = 0,   /**< Operation completed successfully. */
    SPI_ERROR = 1    /**< Invalid argument or bus error.    */
} SPI_Status_t;

/* ── SPI1 pin / port definitions ─────────────────────────────────── */
#define SPI_PORT        GPIOA           /**< GPIO port shared by SCK/MISO/MOSI/CS. */
#define SPI_PIN_SCK     5U              /**< PA5 – Serial Clock.                    */
#define SPI_PIN_MISO    6U              /**< PA6 – Master In Slave Out.             */
#define SPI_PIN_MOSI    7U              /**< PA7 – Master Out Slave In.             */
#define SPI_PIN_CS      9U              /**< PA9 – Chip Select (active-low).        */

/* ── SPI_CR1 bit positions (RM0383 §20.5.1) ─────────────────────── */
#define SPI_CR1_CPHA        (1U  <<  0)  /**< Clock phase.                  */
#define SPI_CR1_CPOL        (1U  <<  1)  /**< Clock polarity.               */
#define SPI_CR1_MSTR        (1U  <<  2)  /**< Master selection.             */
#define SPI_CR1_BR_POS      3U           /**< Baud rate bits [5:3].         */
#define SPI_CR1_BR_MASK     (7U  <<  3)  /**< Baud rate mask.               */
#define SPI_CR1_SPE         (1U  <<  6)  /**< SPI enable.                   */
#define SPI_CR1_LSBFIRST    (1U  <<  7)  /**< Frame format (MSB=0, LSB=1).  */
#define SPI_CR1_SSI         (1U  <<  8)  /**< Internal slave select.        */
#define SPI_CR1_SSM         (1U  <<  9)  /**< Software slave management.    */
#define SPI_CR1_DFF         (1U  << 11)  /**< Data frame format (0=8-bit).  */

/* ── SPI_SR bit positions (RM0383 §20.5.3) ──────────────────────── */
#define SPI_SR_RXNE         (1U  <<  0)  /**< Receive buffer not empty.     */
#define SPI_SR_TXE          (1U  <<  1)  /**< Transmit buffer empty.        */
#define SPI_SR_BSY          (1U  <<  7)  /**< Busy flag.                    */

/* ── Baud-rate prescaler values for BR[2:0] ─────────────────────── */
#define SPI_BR_DIV2         (0U)         /**< fPCLK / 2                     */
#define SPI_BR_DIV4         (1U)         /**< fPCLK / 4                     */
#define SPI_BR_DIV8         (2U)         /**< fPCLK / 8                     */
#define SPI_BR_DIV16        (3U)         /**< fPCLK / 16                    */
#define SPI_BR_DIV32        (4U)         /**< fPCLK / 32 ≈ 3.1 MHz  ← used */
#define SPI_BR_DIV64        (5U)         /**< fPCLK / 64                    */
#define SPI_BR_DIV128       (6U)         /**< fPCLK / 128                   */
#define SPI_BR_DIV256       (7U)         /**< fPCLK / 256                   */

/* ────────────────────────────────────────────────────────────────── */
/*  Public API  (SRS FR-1 … FR-8)                                     */
/* ────────────────────────────────────────────────────────────────── */

/**
 * @brief  Initialize SPI1 and its associated GPIO pins.
 *
 * Configures PA5/PA6/PA7 as AF5 (SCK/MISO/MOSI), PA9 as a
 * push-pull output for Chip Select, enables SPI1 clocking, and
 * writes SPI_CR1 with the default parameters listed in this header.
 * CS is deasserted (high) at the end of init.
 *
 * Satisfies: FR-1, FR-2.
 *
 * @return SPI_OK always (hardware is assumed present).
 */
SPI_Status_t spi_init(void);

/**
 * @brief  Transmit @p length bytes from @p tx_buf over SPI1.
 *
 * Performs polled (blocking) transmission. For each byte the
 * function waits for TXE, writes SPI_DR, then waits for BSY to
 * clear and discards the simultaneously received byte to prevent
 * Overrun (OVR) errors (see RM0383 §20.3.7).
 *
 * Satisfies: FR-3, FR-4.
 *
 * @param[in] tx_buf  Pointer to the source buffer. Must not be NULL.
 * @param[in] length  Number of bytes to send. 0 is a no-op.
 * @return SPI_OK on success, SPI_ERROR if @p tx_buf is NULL.
 */
SPI_Status_t spi_transmit(const uint8_t *tx_buf, uint16_t length);

/**
 * @brief  Receive @p length bytes from a slave device into @p rx_buf.
 *
 * Because SPI is a simultaneous exchange, the master must drive the
 * clock by writing a dummy byte (0x00) to SPI_DR for each byte it
 * wants to read. The function then waits for RXNE and stores the
 * received byte.
 *
 * Satisfies: FR-5, FR-6.
 *
 * @param[out] rx_buf  Pointer to the destination buffer. Must not be NULL.
 * @param[in]  length  Number of bytes to receive. 0 is a no-op.
 * @return SPI_OK on success, SPI_ERROR if @p rx_buf is NULL.
 */
SPI_Status_t spi_receive(uint8_t *rx_buf, uint16_t length);

/**
 * @brief  Assert the Chip Select line (drive PA9 LOW).
 *
 * Must be called before starting a transaction with the slave.
 *
 * Satisfies: FR-7.
 */
void spi_cs_enable(void);

/**
 * @brief  Deassert the Chip Select line (drive PA9 HIGH).
 *
 * Must be called after the last byte of a transaction.
 *
 * Satisfies: FR-8.
 */
void spi_cs_disable(void);

#endif /* SPI_DRIVER_H */
