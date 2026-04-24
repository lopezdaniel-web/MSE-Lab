#include "boton.h"

void btn_init() {
    GPIOC_MODER &= ~(3UL << BTN_MODER);
}

unsigned int btn_leer() {
    return (GPIOC_IDR & (1UL << BTN_PIN)) ? 1 : 0;
}