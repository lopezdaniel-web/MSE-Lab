/**
 * @file    spi_driver.c
 * @brief   Low-level SPI1 driver implementation – STM32F4xx, bare-metal / CMSIS.
 * Polled (blocking) strategy is used for both TX and RX because it
 * keeps the driver simple and deterministic without requiring DMA or
 * interrupt configuration (NFR-1, NFR-3).
 */

#include "spi_driver.h"
#include "gpio_driver.h"

/* ═══════════════════════════════════════════════════════════════════
 * Private helpers
 * ═══════════════════════════════════════════════════════════════════ */

/**
 * @brief Enable the APB2 clock for SPI1 and the AHB1 clock for GPIOA.
 *
 * The GPIOA clock is handled by gpio_driver internally when gpio_init()
 * is called, but SPI1's APB2 gate must be opened here explicitly.
 * (FR-2)
 */
static void spi_enable_clocks(void)
{
    /* SPI1 is on APB2 (RM0383 §6.3.14) */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
}

/**
 * @brief Configure PA5, PA6, PA7 as Alternate Function 5 (SPI1).
 *
 * All three pins: AF mode, push-pull output, very-high speed,
 * no pull-up/pull-down (external termination assumed or not needed
 * at low speed).
 */
static void spi_gpio_init_af(void)
{
    const uint8_t pins[3] = { SPI_PIN_SCK, SPI_PIN_MISO, SPI_PIN_MOSI };

    for (uint8_t i = 0U; i < 3U; i++)
    {
        gpio_init(SPI_PORT, pins[i], GPIO_MODE_AF);
        gpio_set_output_type(SPI_PORT, pins[i], GPIO_OTYPE_PP);
        gpio_set_speed(SPI_PORT, pins[i], GPIO_SPEED_VHIGH);
        gpio_set_pupd(SPI_PORT, pins[i], GPIO_PUPD_NONE);
        gpio_set_af(SPI_PORT, pins[i], GPIO_AF5);   /* SPI1 alternate function */
    }
}

/**
 * @brief Configure PA9 as a push-pull output for Chip Select.
 *
 * CS starts HIGH (deasserted) so no unintentional transaction is
 * triggered during or after initialization.
 */
static void spi_gpio_init_cs(void)
{
    gpio_init(SPI_PORT, SPI_PIN_CS, GPIO_MODE_OUTPUT);
    gpio_set_output_type(SPI_PORT, SPI_PIN_CS, GPIO_OTYPE_PP);
    gpio_set_speed(SPI_PORT, SPI_PIN_CS, GPIO_SPEED_VHIGH);
    gpio_set_pupd(SPI_PORT, SPI_PIN_CS, GPIO_PUPD_NONE);
    gpio_write(SPI_PORT, SPI_PIN_CS, 1U);           /* CS deasserted (high)    */
}

/**
 * @brief Write SPI_CR1 with the default operating parameters.
 *
 * Bit-field breakdown written to SPI1->CR1:
 *   MSTR     = 1  → Master mode
 *   SSM      = 1  → Software NSS management
 *   SSI      = 1  → Internal NSS tied high (required for master when SSM=1)
 *   BR[2:0]  = 4  → Prescaler /32 → ~3.125 MHz  (APB2 @ 100 MHz)
 *   CPOL     = 1  → Clock idles HIGH  (Mode 3)
 *   CPHA     = 1  → Data captured on second (rising) edge
 *   DFF      = 0  → 8-bit data frame
 *   LSBFIRST = 0  → MSB transmitted first
 *   SPE      = 0  → Peripheral disabled during setup; enabled at the end
 *
 * SPE is set last to ensure all parameters are stable before the
 * peripheral is activated (RM0383 §20.3.3).
 */
static void spi_peripheral_configure(void)
{
    /* Ensure SPI1 is disabled before configuration */
    SPI1->CR1 &= ~SPI_CR1_SPE;

    /* Clear CR1 and CR2 to a known state */
    SPI1->CR1 = 0U;
    SPI1->CR2 = 0U;

    /* Build CR1 value */
    SPI1->CR1 |= SPI_CR1_MSTR;                                 /* Master mode        */
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;                    /* Software NSS       */
    SPI1->CR1 |= ((uint32_t)SPI_BR_DIV32 << SPI_CR1_BR_POS);  /* Baud /32 ≈ 3 MHz   */
    SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA;                  /* Mode 3             */
    /* DFF=0 (8-bit), LSBFIRST=0 (MSB first) – already cleared above */

    /* Enable the SPI peripheral (FR-1) */
    SPI1->CR1 |= SPI_CR1_SPE;
}

/* ═══════════════════════════════════════════════════════════════════
 * Public API implementation
 * ═══════════════════════════════════════════════════════════════════ */

/* ── spi_init ─────────────────────────────────────────────────────── */
SPI_Status_t spi_init(void)
{
    spi_enable_clocks();          /* Step 1: clock the peripheral (FR-2) */
    spi_gpio_init_af();           /* Step 2: configure SCK/MISO/MOSI     */
    spi_gpio_init_cs();           /* Step 3: configure CS as GPIO output  */
    spi_peripheral_configure();   /* Step 4 + 5: set parameters, enable   */
    return SPI_OK;
}

/* ── spi_transmit ────────────────────────────────────────────────── */
SPI_Status_t spi_transmit(const uint8_t *tx_buf, uint16_t length)
{
    /* NFR-2: validate pointer */
    if (tx_buf == NULL)
    {
        return SPI_ERROR;
    }

    for (uint16_t i = 0U; i < length; i++)
    {
        /* FR-4 – Wait until TXE=1 (Transmit Buffer Empty) */
        while (!(SPI1->SR & SPI_SR_TXE))
        {
            // wait
        }

        /* FR-3 – Load next byte into the data register (Tx buffer) */
        SPI1->DR = (uint32_t)tx_buf[i];

        /* FR-4 – Wait until BSY=0 (transfer fully complete on the wire) */
        while (SPI1->SR & SPI_SR_BSY)
        {
            // wait
        }
        (void)SPI1->DR;
        (void)SPI1->SR;
    }

    return SPI_OK;
}

/* ── spi_receive ──────────────────────────────────────────────────── */
SPI_Status_t spi_receive(uint8_t *rx_buf, uint16_t length)
{
    /* NFR-2: validate pointer */
    if (rx_buf == NULL)
    {
        return SPI_ERROR;
    }

    for (uint16_t i = 0U; i < length; i++)
    {
        SPI1->DR = 0x00U;

        /* FR-5 – Wait until RXNE=1 (Receive Buffer Not Empty) */
        while (!(SPI1->SR & SPI_SR_RXNE))
        {
            // wait
        }

        /* FR-5 – Read received byte from the data register (Rx buffer) */
        rx_buf[i] = (uint8_t)SPI1->DR;
    }

    return SPI_OK;
}

/* ── spi_cs_enable ────────────────────────────────────────────────── */
void spi_cs_enable(void)
{
    /* FR-7 – Drive CS LOW to select (assert) the target device */
    gpio_write(SPI_PORT, SPI_PIN_CS, 0U);
}

/* ── spi_cs_disable ───────────────────────────────────────────────── */
void spi_cs_disable(void)
{
    /* FR-8 – Drive CS HIGH to deselect (deassert) the target device */
    gpio_write(SPI_PORT, SPI_PIN_CS, 1U);
}
