#include "gpio_driver.h"
#include "uart_driver.h"
#include "utils.h"
#include "serial.h"
#include <stdarg.h>


void serial_init(void)
{
    uart_init(UART_BAUD_115200);   // Initialize USART2 with the BRR value for 115200 baud
}

void serial_printf(const char *format, ...)
{
    char buffer[SERIAL_BUF_SIZE];           // Local buffer to hold the formatted string
    va_list args;                           // Variable to handle the variable arguments

    va_start(args, format);                 // Initialize the variable args with the last fixed parameter (format)

    utils_vsnprintf(buffer, format, args);   // Format the string into the buffer using utils_snprintf

    va_end(args);                           // Clean up the variable argument list

    // Transmit each character in the buffer using uart_write
    for (size_t i = 0; buffer[i] != '\0'; i++)
    {
        uart_write((uint8_t)buffer[i]);     // Send each character until the null terminator is reached
    }
}