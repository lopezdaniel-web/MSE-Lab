/**
 * @file    main.c
 * @brief   Application entry point – ADC + PWM sensor-controlled demo.
 *          NUCLEO-F411RE, Lab Assignment 03.
 *
 * ── What this does ───────────────────────────────────────────────────
 *
 *   1. PA1  (Sensor) – reads a potentiometer voltage via ADC1 CH1.
 *                      Raw range: 0–4095  (12-bit, 0 V – 3.3 V)
 *
 *   2. PA8  (PWM)    – 1 kHz PWM signal on TIM1 CH1.
 *                      Duty cycle is mapped from the ADC reading:
 *                        duty (%) = adc_raw * 100 / 4095
 *                      Turning the potentiometer varies LED brightness.
 *
 *   3. PA5  (LD2)    – heartbeat: toggles every 100 ms to show the
 *                      super-loop is running.
 *
 * ── Mapping: ADC → PWM duty cycle ───────────────────────────────────
 *
 *   ADC =    0  →  duty =   0 %  →  LED fully off
 *   ADC = 2047  →  duty =  50 %  →  LED half brightness
 *   ADC = 4095  →  duty = 100 %  →  LED fully on
 *
 *   Formula (integer arithmetic, no floating-point):
 *     duty_pct = (uint8_t)((adc_raw * 100UL) / 4095UL)
 *
 * ── Clock assumptions ────────────────────────────────────────────────
 *
 *   HSI 16 MHz, all AHB/APB prescalers = 1 (default after reset).
 *   TIM2 → 1 ms delay base   (APB1, PSC=15, ARR=999)
 *   TIM1 → 1 kHz PWM         (APB2, PSC=15, ARR=999)
 *   ADC1 → 8 MHz ADCCLK      (APB2/2)
 *
 * ── Verification ─────────────────────────────────────────────────────
 *
 *   Sensor:   connect multimeter to PA1; rotate potentiometer full range.
 *   PWM:      oscilloscope on PA8 (CN10 pin 23) – observe duty changing.
 *   LED:      LD2 on board (PA8 via external LED or direct observation
 *             of PWM on scope confirms brightness modulation).
 *   Heartbeat: LD2 (PA5) blinks at ~5 Hz to confirm loop is alive.
 */

#include "stm32f411xe.h"
#include "led.h"
#include "timer.h"
#include "pwm.h"
#include "sensor.h"

/* ====================================================================
 * Application Constants
 * ==================================================================== */

#define APP_PWM_FREQUENCY_HZ    (1000U)  /**< PWM carrier frequency [Hz]    */
#define APP_ADC_MAX_VALUE       (4095UL) /**< 12-bit ADC full-scale value    */
#define APP_HEARTBEAT_MS        (100U)   /**< LED heartbeat toggle period    */

/* ====================================================================
 * Private – ADC to PWM duty cycle mapping
 * ==================================================================== */

/**
 * @brief  Map a 12-bit ADC reading to a 0–100 % duty cycle.
 *
 *   duty_pct = (adc_raw × 100) / 4095
 *
 * Uses 32-bit intermediate to avoid overflow:
 *   max: 4095 × 100 = 409 500  which fits in uint32_t.
 *
 * @param  adc_raw  Raw ADC result (0–4095).
 * @return Duty cycle percentage (0–100).
 */
static uint8_t map_adc_to_duty(uint16_t adc_raw)
{
    uint32_t duty = ((uint32_t)adc_raw * 100UL) / APP_ADC_MAX_VALUE;

    if (duty > 100U) { duty = 100U; }  /* clamp – should never trigger */

    return (uint8_t)duty;
}

/* ====================================================================
 * Entry Point
 * ==================================================================== */

int main(void)
{
    uint16_t adc_raw;
    uint8_t  duty_pct;

    /* ── 1. Heartbeat LED on PA5 ────────────────────────────────── */
    led_init();

    /* ── 2. TIM2 for blocking delays ────────────────────────────── */
    timer_init();

    /* ── 3. TIM1 CH1 on PA8: 1 kHz PWM, start at 0 % duty ──────── */
    pwm_init(APP_PWM_FREQUENCY_HZ);
    pwm_set_signal(0U);
    pwm_start();

    /* ── 4. ADC1 CH1 on PA1: potentiometer input ────────────────── */
    sensor_init();

    /* ── 5. Super-loop ──────────────────────────────────────────── */
    /*
     * Each iteration:
     *   a. Trigger one ADC conversion.
     *   b. Read the 12-bit result (blocks until EOC).
     *   c. Map the result to a 0–100 % duty cycle.
     *   d. Update the PWM duty cycle.
     *   e. Toggle heartbeat LED.
     *   f. Wait 100 ms (sets the loop rate to ~10 conversions/second).
     */
    while (1)
    {
        /* a. Start conversion */
        sensor_start_conversion();

        /* b. Read result (polls SR.EOC internally) */
        adc_raw = sensor_read_value();

        /* c. Map ADC reading → duty cycle percentage */
        duty_pct = map_adc_to_duty(adc_raw);

        /* d. Apply new duty cycle to TIM1 CH1 */
        pwm_set_signal(duty_pct);

        /* e. Heartbeat */
        led_toggle();

        /* f. Loop delay – also gives the ADC enough time to settle */
        timer_delay_ms(APP_HEARTBEAT_MS);
    }

    /* Unreachable – satisfies compiler warning */
    return 0;
}
