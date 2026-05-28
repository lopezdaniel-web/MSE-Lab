/**
 * @file    uart_driver.c
 * @brief   Low-level UART driver – USART2 on STM32F411RE (CMSIS).
 *
 * Register map used:
 *   RCC->APB1ENR  [17]      USART2EN  – peripheral clock enable
 *   RCC->AHB1ENR  [0]       GPIOAEN   – GPIO clock (shared, may already be on)
 *   GPIOA->MODER  [5:4]     PA2 = AF  (10)
 *   GPIOA->MODER  [7:6]     PA3 = AF  (10)
 *   GPIOA->AFR[0] [11:8]    PA2 AF7 = USART2_TX
 *   GPIOA->AFR[0] [15:12]   PA3 AF7 = USART2_RX
 *   USART2->BRR   [15:0]    baud rate divisor
 *   USART2->CR1   [3]  TE   transmitter enable
 *   USART2->CR1   [2]  RE   receiver enable
 *   USART2->CR1   [13] UE   USART enable
 *   USART2->SR    [7]  TXE  TX data register empty flag
 *   USART2->DR    [7:0]     transmit / receive data register
 */

#include "uart_driver.h"
#include "gpio_driver.h"

/* ── uart_init ───────────────────────────────────────────────────── */

/**
 * @brief  Initialise USART2 for asynchronous TX at the given baud rate.
 *
 * Sequence:
 *  1. Enable GPIOA clock and configure PA2 (TX) and PA3 (RX) as AF7.
 *  2. Enable USART2 clock on APB1.
 *  3. Write BRR with the pre-calculated divisor.
 *  4. Set CR1: TE=1 (transmitter), RE=1 (receiver), UE=1 (enable).
 *     Note: UE must be set AFTER TE/RE per the reference manual.
 */
uart_status_t uart_init(uint32_t brr)
{
    /* 1. GPIO – PA2 TX, PA3 RX, both AF7 (USART2) */
    gpio_init(GPIOA, 2U, GPIO_MODE_AF);
    gpio_set_output_type(GPIOA, 2U, GPIO_OTYPE_PP);
    gpio_set_speed(GPIOA, 2U, GPIO_SPEED_HIGH);
    gpio_set_pupd(GPIOA, 2U, GPIO_PUPD_NONE);
    gpio_set_af(GPIOA, 2U, GPIO_AF7);

    gpio_init(GPIOA, 3U, GPIO_MODE_AF);
    gpio_set_output_type(GPIOA, 3U, GPIO_OTYPE_PP);
    gpio_set_speed(GPIOA, 3U, GPIO_SPEED_HIGH);
    gpio_set_pupd(GPIOA, 3U, GPIO_PUPD_PULLUP);  /* idle line = HIGH */
    gpio_set_af(GPIOA, 3U, GPIO_AF7);

    /* 2. Enable USART2 peripheral clock on APB1 */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* 3. Baud rate divisor */
    USART2->BRR = brr;

    /* 4. Enable transmitter, receiver, and USART peripheral */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    return UART_OK;
}

/* ── uart_write ──────────────────────────────────────────────────── */

/**
 * @brief  Send one byte over USART2.
 *
 *  FR-4: Poll SR.TXE until the transmit data register is empty.
 *  FR-5: Write byte to DR – hardware starts transmission immediately.
 *
 *  TXE is set by hardware when DR is transferred to the shift register.
 *  It does NOT mean the byte has finished transmitting; use TC if that
 *  level of synchronisation is needed (not required by this SRS).
 */
void uart_write(uint8_t byte)
{
    /* Wait until TXE = 1 (DR is empty and ready to accept new data) */
    while (!(USART2->SR & USART_SR_TXE));

    /* Write to DR – clears TXE and begins transmission */
    USART2->DR = (uint32_t)(byte & 0xFFU);
}