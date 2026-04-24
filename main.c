#include "gpio_driver.h"
#include "led.h"
#include "boton.h"

#define DEBOUNCE 50000U

int main(void)
{
    gpio_init();
    led_init();
    btn_init();

    unsigned int btn_prev  = 1;
    unsigned int led_state = 0;
    unsigned int debounce  = 0;

    for(;;)
    {
        unsigned int btn_now = btn_leer();

        if (btn_prev == 1 && btn_now == 0 && debounce == 0)
        {
            led_state ^= 1;

            if (led_state)
                encender();
            else
                apagar();

            debounce = DEBOUNCE;
        }

        if (debounce > 0)
            debounce--;

        btn_prev = btn_now;
    }
}