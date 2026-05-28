/**
 * @file    main.c
 * @brief   Lab 04 – UART telemetry – NUCLEO-F411RE.
 *
 * Hardware:
 *   PA1  ← potentiometer wiper        (ADC1_IN1)
 *   PA0  → external LED + 330Ω GND   (TIM2_CH1 PWM, 1 kHz)
 *   PA2  → ST-Link virtual COM TX     (USART2_TX, AF7)
 *   PA5  → on-board LD2               (heartbeat blink)
 *
 * Timer allocation:
 *   TIM2 → PWM on PA0   (APB1)
 *   TIM5 → 500 ms delay (APB1)
 *
 * Loop behaviour (every 500 ms):
 *   1. Read potentiometer via ADC1_IN1 → raw 0–4095.
 *   2. Convert raw to millivolts: mV = raw × 3300 / 4095.
 *   3. Map raw to duty cycle 0–100 % → update PWM.
 *   4. Send telemetry string over USART2:
 *        "ADC Value: 2047 | Voltage: 1650 mV\n"
 *   5. Toggle LD2 on PA5.
 *
 * Serial Monitor settings (VS Code):
 *   Port     : ST-Link COM port (COMx on Windows)
 *   Baud rate: 115200
 */

#include "stm32f411xe.h"
#include "led.h"
#include "timer.h"
#include "sensor.h"
#include "serial.h"

#define REPORT_DELAY_MS (500U)    /* Telemetry period            */
#define VREF_MV         (3300U)   /* Reference voltage in mV     */
#define ADC_FULL_SCALE  (4095U)   /* 12-bit max value            */

int main(void)
{
    led_init();                     /* PA5 → output               */
    timer_init();                   /* TIM5 clock enable          */
    sensor_init();                  /* PA1 analog, ADC1 channel 1 */
    serial_init();                  /* USART2 at 115200 baud      */

    while (1)
    {
        /* 1. Read ADC */
        sensor_start_conversion();
        uint16_t raw  = sensor_read_value();             /* 0–4095  */

        /* 2. Convert to millivolts: mV = raw × 3300 / 4095 */
        uint32_t mv   = ((uint32_t)raw * VREF_MV) / ADC_FULL_SCALE;

        /* 4. Send telemetry over USART2 */
        serial_printf("ADC Value: %d | Voltage: %d mV\r\n",(int)raw, (int)mv);

        /* 5. Heartbeat */
        led_toggle();

        /* 6. Wait 500 ms before next report */
        timer_delay_ms(REPORT_DELAY_MS);
    }

    return 0;
}