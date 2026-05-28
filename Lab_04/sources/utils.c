/******************************************************************************
 * Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Carlos Villarreal and CETYS Universidad are not liable for any
 * misuse of this material.
 *
 *****************************************************************************/
/**
 * @file utils.c
 * @brief Utility library with helper functions.
 *
 * Utils module has helper functions to treat strings, ASCII conversions, and
 * printing utilities.
 *
 * @author Your Name
 * @date 04/30/2026
 *
 */

/*** Includes ***/
#include "utils.h"

/*** Preprocessor Definitions ***/

/*** Type Prototypes ***/

/*** Local Variables ***/

/*** External Variables ***/

/*** Function Prototypes ***/

static uint32_t utils_printString(char *dst, char *src);
static uint32_t utils_printInt(char *dst, int32_t num, uint8_t sign, uint32_t base);

/*** Function Definitions ***/

void utils_vsnprintf(char *dst, const char *format, va_list args)
{
    /* No longer using it since we define it in serial to avoid losing va_list when calling utils_printInt and utils_printString 
    va_list args;                Creates the variable to handle the variable arguments
    va_start(args, format);      Initializes the variable args
    */

    // Starts reading the format string character by character
    while (*format)
    {
        // Checks for the % symbol
        if (*format == '%')
        {
            format++;                                                       // Aumenta el apuntador format
            uint32_t len = 0;                                               // Variable para sacar la longitud de cada argumento variable (args) 
            switch (*format)
            {
                /* Hint: on the data type cases use va_arg(args, data_type) */
                case 's':
                    // Caso de strings

                    len = utils_printString(dst, va_arg(args, char*));         // Copia el string que hay en args y se manda a dst

                    dst += len;                                                // avanzar el puntero los espacios que ya se llenaron en dst con el string
                    break;
                case 'd':
                    // Caso de int                    
                    len = utils_printInt(dst, va_arg(args, int), 1, 10);       // Convertir el numero int en va_arg a un string y meterlo a dst

                    dst += len;                                                // avanzar el puntero los espacios que ya se llenaron en dst con el int
                    break;
                case 'u':

                    // Caso unsinged int
                    len = utils_printInt(dst, va_arg(args, unsigned int), 0, 10);    // Convertir el numero unsigned int en va_arg a un string y meterlo a dst

                    dst += len;                                                      // avanzar el puntero los espacios que ya se llenaron en dst con el unsigned int
                    break;
                case 'x':

                    //  Caso Hexadecimal
                    len = utils_printInt(dst, va_arg(args, unsigned int), 0, 16);     // Convertir el numero int en va_arg a un string y meterlo a dst

                    dst += len;                                                       // avanzar el puntero los espacios que ya se llenaron en dst con el int
                    break;
                case 'c':
                    // Caso char
                    *dst++ = (char)va_arg(args, int);                                 // Se le mete el valor en int que hay en args y se transforma a char
                    break;
                case '%':
                    *dst++ = '%';
                    break;
                default:
                    *dst++ = '%';                                                   // Mete a dst un %
                    *dst++ = *format;                                               // Mete a dst el caracter incorrecto que se metio
                    break;
            }
        }
        else
        {
            *dst++ = *format;
        }

        format++;
    }
    *dst = '\0';   //Agregar un final del string poniendo un NULL
}
/* New funtion so we can call it from serial without losing va_list since utils_p */
void utils_snprintf(char *dst, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    utils_vsnprintf(dst, format, args);
    va_end(args);
}

/**
 * @brief Copy a string into a destination buffer.
 *
 * This function calculates the length of the source string and copies
 * its contents into the destination buffer.
 *
 * @param dst Pointer to the destination buffer where the string will be copied.
 * @param src Pointer to the source string to copy.
 *
 * @return Length of the string copied (number of characters).
 */
static uint32_t utils_printString(char *dst, char *src)
{
    uint32_t length = 0;     // Variable to count the length of the string copied

    while (*src)             // while the current character in the source string is not the null terminator
    {
        *dst++ = *src++;     // Copies the current character from the source string to the destination buffer and advances both pointers
        length++;            // Increment the length counter for each character copied
    }

    return length;           // Returns the total length of the string copied
}

/**
 * @brief Convert an integer to ASCII and copy it into a destination buffer.
 *
 * This function converts an integer into its ASCII representation
 * based on the specified base (2-16) and sign option.
 * The resulting string is copied into the destination buffer.
 *
 * @param dst Pointer to the destination buffer where the ASCII string will be stored.
 * @param num Integer number to convert.
 * @param sign Interger value that indicates if data is signed or unsigned.
 * @param base Numerical base for conversion.
 *
 * @return Length of the ASCII string copied into the destination buffer.
 */
static uint32_t utils_printInt(char *dst, int32_t num, uint8_t sign, uint32_t base)
{
    uint8_t buffer[33];                                         // Buffer to hold the converted number (32 digits + null terminator)
    uint32_t length = utils_itoa(num, buffer, sign, base);      // Length of the converted string
    
    utils_memCpy(dst, buffer, length);                          // Copy the converted string to the destination buffer
    
    return length;                                              // Return the length of the converted string
}

/**
 * @brief Copy a block of memory from source to destination.
 *
 * This function copies a specified number of bytes from the source buffer
 * to the destination buffer. It assumes that the source and destination
 * buffers do not overlap.
 *
 * @param dst Pointer to the destination buffer where data will be copied.
 * @param src Pointer to the source buffer containing the data to copy.
 * @param length Number of bytes to copy from source to destination.
 *
 * @return Pointer to the destination buffer (dst).
 */
void * utils_memCpy(void *dst, void *src, size_t length)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    while (length--)
    {
        *d++ = *s++;
    }

    return dst;
}

/**
 * @brief Reverse the order of elements of a data set
 *
 * This function takes a pointer to a data set and its length, and reverses
 * the order of the elements in place.
 *
 * @param data Pointer to the data set to reverse.
 * @param length Number of bytes in the data set.
 *
 * @return Pointer to the reversed data set (data).
 */
void * utils_memReverse(void *data, size_t length)
{
    size_t end = length - 1;                                    // Index of the last element
    for(size_t i = 0; i < end; i++, end--)                      // Loop until the middle of the data set
    {
        uint8_t temp = *((uint8_t *)data + i);                  // Store the current element in a temporary variable
        *((uint8_t *)data + i) = *((uint8_t *)data + end);      // Move the element from the end to the current position
        *((uint8_t *)data + end) = temp;                        // Move the temp/original variable to the end position   
    }
   return data;                                                 // Return the pointer to the reversed data set
}

/**
 * @brief Convert data from integer type into an ASCII string
 *
 * Given an integer value, this will convert a provided integer to
 * an ASCII string data type regardless of the integer base (2-16),
 * and return the number of digits of the converted ASCII string.
 * The numerical system of the integer is determined by the provided
 * base value.
 *
 * @param data Integer value to convert to
 * @param ptr Pointer to the ASCII string
 * @param sign Interger value that indicates if data is signed or unsigned
 * @param base Base of the integer to convert to
 *
 * @return Length of the converted data.
 */
uint32_t utils_itoa(int32_t data, uint8_t *ptr, uint8_t sign, uint8_t base) 
{
    uint32_t i = 0;                                         // Index for the ASCII string
    uint32_t num = 0;                                       // Aux variable to hold the number to convert
    uint8_t negative = 0;                                   // Flag to know if the number is negative

    // Si el numero es negativo
    if (sign == 1 && (data < 0)) 
    {
        negative = 1;                                       // Negative flag is set to 1
        num = (uint32_t)(-data);                            // Change the number to positive for conversion
    } 
    else 
    {
        num = (uint32_t)data;                               // In any other case, simply assign the number to num
    }

    // Special case for 0
    if (num == 0)
    {
        ptr[i++] = '0';
    }

    // While number > 0, keep converting 
    while (num > 0)
    {
        uint32_t residuo = num % base;                      // Obtener el residuo de num y la base del numero al que se quiere convertir

        // Si es digito decimal 
        if (residuo < 10) 
        {
            ptr[i++] = (uint8_t)(residuo + '0');            // Turn to ASCII by adding '0'
        } 
        // Si es un numero Hexadecimal mayor a 10 
        else 
        {
            ptr[i++] = (uint8_t)((residuo - 10) + 'A');     // Turn to HEX
        }

        num /= base;                                        // Divide number by the base 
    }

    if (negative) 
    {
        ptr[i++] = '-';                                     // Add negative sign if the number was negative 
    }
    utils_memReverse(ptr, i);                               // Reverse the string to get the correct order
    ptr[i] = '\0';                                          // End string

    return i; 


}


/**
 * @brief Convert data from an ASCII string into an integer type
 *
 * Given an unsigned integer pointer, this will convert a pointer to
 * a character string to an integer data type regardless of the integer
 * base (2-16), and return the converted integer data.
 * The numerical system of the integer is determined by the provided
 * base value and the number of characters is determined by the provided
 * digits value.
 *
 * @param ptr Pointer to the ASCII string
 * @param digits Number of digits in the ASCII string
 * @param sign Interger value that indicates if data is signed or unsigned
 * @param base Base of the converted integer
 *
 * @return Converted integer value.
 */
int32_t utils_atoi(uint8_t *ptr, uint32_t digits, uint8_t sign, uint8_t base)
{
    int32_t result = 0;     // Final result variable
    uint32_t i = 0;         // Index
    uint8_t negative = 0;   // Flag to know if the number is negative 

    // Check for valid base
    if (base < 2 || base > 16)
    {
        return 0;
    }

    // Verify sign and set negative flag if necessary
    if (sign == 1 && ptr[0] == '-')
    {
        negative = 1;
        i = 1;              // Start after the negative sign for conversion
    }

    // Step through each char
    for (; i < digits; i++)
    {
        uint8_t c = ptr[i];
        uint32_t value = 0;

        // Check for valid digit based on base
        if (c >= '0' && c <= '9')
        {
            value = c - '0';
        }
        // Range from A to F for hexadecimal
        else if (c >= 'A' && c <= 'F')
        {
            value = c - 'A' + 10;
        }
        // If case for invalid base
        else
        {
            break; 
        }

        // Check if value is valid for the base
        if (value >= base)
        {
            break;
        }

        // Build the result
        result = result * base + value;
    }

    // Apply negative sign if needed
    if (negative)
    {
        result = -result;
    }

    return result;
}