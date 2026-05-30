#include "LCD_driver.h"
#include "gpio_driver.h"
#include "timer.h"

/* ═══════════════════════════════════════════════════════════════════
 * Private helpers
 * ═══════════════════════════════════════════════════════════════════ */

static void lcd_delay(void)
{
    timer_delay_ms(2U);
}

/* ═══════════════════════════════════════════════════════════════════
 * Public API
 * ═══════════════════════════════════════════════════════════════════ 
*/
static void lcd_pulse_enable(void)
{
    gpio_write(LCD_PORT, LCD_PIN_E, 1U);
    lcd_delay();   /* Ensure minimum pulse width is met */
    gpio_write(LCD_PORT, LCD_PIN_E, 0U);
    lcd_delay();   /* Ensure minimum pulse width is met */
}

static void lcd_sNibble_E(uint8_t nibble, uint8_t rs)
{
    /* Set RS pin according to rs argument */
    gpio_write(LCD_PORT, LCD_PIN_RS, rs);

    /* Set data pins D4–D7 according to the nibble */
    gpio_write(LCD_PORT, LCD_PIN_D4, (nibble >> 0) & 0x01U);
    gpio_write(LCD_PORT, LCD_PIN_D5, (nibble >> 1) & 0x01U);
    gpio_write(LCD_PORT, LCD_PIN_D6, (nibble >> 2) & 0x01U);
    gpio_write(LCD_PORT, LCD_PIN_D7, (nibble >> 3) & 0x01U);

    lcd_pulse_enable();
}

static void lcd_send_byte(uint8_t byte, uint8_t rs)
{
    /* Send upper nibble */
    lcd_sNibble_E(byte >> 4, rs);

    /* Send lower nibble */
    lcd_sNibble_E(byte & 0x0FU, rs);
}

static void lcd_send_command(uint8_t cmd)
{
    lcd_send_byte(cmd, 0U);   /* RS=0 for command */
}

static void lcd_send_data(uint8_t data)
{
    lcd_send_byte(data, 1U);  /* RS=1 for data */
}

void lcd_init(void)
{
    const uint8_t pins[] = 
    { 
        LCD_PIN_RS,
        LCD_PIN_E,
        LCD_PIN_D4,
        LCD_PIN_D5,
        LCD_PIN_D6,
        LCD_PIN_D7
    };

    for (uint8_t i = 0U; i < 6U; i++)
    {
        gpio_init(LCD_PORT, pins[i], GPIO_MODE_OUTPUT);
        gpio_set_output_type(LCD_PORT, pins[i], GPIO_OTYPE_PP);
        gpio_set_speed(LCD_PORT, pins[i], GPIO_SPEED_MEDIUM);
        gpio_set_pupd(LCD_PORT, pins[i], GPIO_PUPD_NONE);
        gpio_write(LCD_PORT, pins[i], 0U);  /* All lines start LOW */
    }

    timer_delay_ms(20U);    /* esperar VDD estable */

    /* 3 pulsos de arranque */
    lcd_sNibble_E(0x03U, 0U);
    timer_delay_ms(5U);
    lcd_sNibble_E(0x03U, 0U);
    lcd_delay();
    lcd_sNibble_E(0x03U, 0U);
    lcd_delay();

    /* switch a 4-bit */
    lcd_sNibble_E(0x02U, 0U);
    lcd_delay();

    /* configuración */
    lcd_send_command(0x28U);    /* 4-bit, 2 líneas, 5x8 */
    lcd_send_command(0x08U);    /* display off */
    lcd_delay();
    lcd_send_command(0x01U);    /* clear */
    lcd_send_command(0x06U);    /* entry mode */
    lcd_send_command(0x0CU);    /* display on */
    lcd_delay();
}

void lcd_print_char(char ch)
{
    lcd_send_data((uint8_t)ch);
}

LCD_Status_t lcd_print_string(const char *str) 
{
    while (*str != '\0')
    {
        lcd_print_char(*str);
        str++;
    }
    return LCD_OK;    
}

void lcd_clear(void)
{
    for (uint8_t i = 0U; i < LCD_COLS; i++)
    {
        lcd_print_char(' ');
    }
}

LCD_Status_t lcd_set_cursor(uint8_t row, uint8_t col)
{
    if ((row >= LCD_ROWS) || (col >= LCD_COLS))
    {
        return LCD_ERROR;
    }

    uint8_t addr = (row == 0U) ? (LCD_ROW0_ADDR + col) : (LCD_ROW1_ADDR + col);
    lcd_send_command(0x80U | addr);
    return LCD_OK;
}