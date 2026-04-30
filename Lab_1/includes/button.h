#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

/* Logical state values returned by button_get_state() */
#define BUTTON_PRESSED    1U
#define BUTTON_RELEASED   0U

void    button_init      (void);
uint8_t button_get_state (void);

#endif /* BUTTON_H */