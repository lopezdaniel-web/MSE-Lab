#include "timer.h"
#include "serial.h"
#include "utils.h"
#include "LCD_driver.h"
#include "mpu6050.h"

/*
int main(void)
{
    // Original main, basic LCD test
    timer_init();
    lcd_init();

    lcd_print_string("Hello World!");

    while (1) { }
    
    // 2nd main, User input from UART to LCD
    timer_init();
    serial_init();
    lcd_init();

    char    buf[17];
    uint8_t idx = 0U;

    while (1)
    {
        if (uart_available())
        {
            char ch = (char)uart_read();

            if (ch == '\r' || ch == '\n')
            {
                lcd_clear();
                buf[idx] = '\0';
                lcd_set_cursor(0U, 0U);
                lcd_print_string(buf);
                serial_printf("buf: %s\r\n", buf);
                idx = 0U;
            }
            else if (idx < 16U)
            {
                buf[idx] = ch;
                idx++;
            }
        }
    }
}
*/
/**
 * @file    main.c
 * @brief   MPU6050 → LCD demo.
 *
 *   Row 0:  "X:XXXXX Y:XXXXX"
 *   Row 1:  "Z:XXXXX"
 *
 * Hardware:
 *   LCD     – PB5/PB6/PB7/PB10/PB12/PB13  (4-bit parallel)
 *   MPU6050 – PB8 (SCL), PB9 (SDA)        (I2C1, AF4)
 */

int main(void)
{
    MPU6050_t sensor;
    char row0[17];
    char row1[17];

    timer_init();
    serial_init();
    mpu6050_init();
    lcd_init();

    serial_printf("Ready\r\n");

    while (1)
    {
        mpu6050_readData(&sensor);

        utils_snprintf(row0, "X:%d Y:%d", sensor.gx, sensor.gy);
        utils_snprintf(row1, "Z:%d", sensor.gz);

        lcd_set_cursor(0U, 0U);
        lcd_clear();
        lcd_set_cursor(0U, 0U);
        lcd_print_string(row0);

        lcd_set_cursor(1U, 0U);
        lcd_clear();
        lcd_set_cursor(1U, 0U);
        lcd_print_string(row1);

        serial_printf("GX:%d GY:%d GZ:%d\r\n",sensor.gx, sensor.gy, sensor.gz);

        timer_delay_ms(500U);
    }
}