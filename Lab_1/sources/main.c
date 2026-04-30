/**
 ******************************************************************************
 * @file    main.c
 * @author  Paez Sandoval Jose Manuel - T037945
 * @brief   GPIO lab — toggle LD2 on each B1 press (Nucleo-F411RE)
 *          Debounce is handled entirely inside button_get_state() via
 *          an integrator; no blocking delays anywhere in the loop.
 ******************************************************************************
 * Copyright (C) 2026 by Jose Paez - CETYS Universidad
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Jose Paez and CETYS Universidad are not liable for any
 * misuse of this material.
 * 
 * Hardware:
 *   LED  LD2  → PA5  (output, active-high)
 *   Button B1 → PC13 (input,  active-low, external 4.7 kΩ pull-up)
 ******************************************************************************
 */

#include <stdint.h>
#include "led.h"
#include "button.h"

int main(void)
{
    led_init();
    button_init();

    uint8_t last_state = BUTTON_RELEASED;

    for (;;)
    {
        uint8_t current_state = button_get_state();

        /* Rising edge: released → pressed */
        if (current_state == BUTTON_PRESSED && last_state == BUTTON_RELEASED)
        {
            led_toggle();
        }

        last_state = current_state;
        /* No delay — integrator inside button_get_state() does the filtering */
    }
}