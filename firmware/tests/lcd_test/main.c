/* Flick & Fly: ALS LPC1768 trainer 16x2 LCD diagnostic.
 * Trainer manual pp26,31-32: CND -> CNAD -> CN11, four-bit LCD data.
 * Lab 9 pp39-41: P0.23-P0.26 = D4-D7, P0.27 = RS, P0.28 = EN;
 * JP5 and JP16 shorted, POT3 adjusts contrast. No R/W control is used.
 * The installed startup_LPC17xx.s calls SystemInit before main.
 */
#include <LPC17xx.h>

#define LCD_DATA 0x07800000UL
#define LCD_RS   0x08000000UL
#define LCD_EN   0x10000000UL
#define LCD_PINS (LCD_DATA | LCD_RS | LCD_EN)

static void lcd_delay(unsigned long count)
{
    volatile unsigned long i;
    /* Conservative software delay following Lab 9; not a calibrated timer. */
    for (i = 0UL; i < count; ++i) {
    }
}

static void lcd_nibble(unsigned int nibble, unsigned int data_mode)
{
    LPC_GPIO0->FIOCLR = LCD_DATA | LCD_RS | LCD_EN;
    LPC_GPIO0->FIOSET = ((unsigned long)(nibble & 0x0FU) << 23);
    if (data_mode != 0U) LPC_GPIO0->FIOSET = LCD_RS;
    lcd_delay(1000UL);
    LPC_GPIO0->FIOSET = LCD_EN;
    lcd_delay(1000UL);
    LPC_GPIO0->FIOCLR = LCD_EN;
    lcd_delay(30000UL);
}

static void lcd_byte(unsigned int value, unsigned int data_mode)
{
    lcd_nibble((value >> 4) & 0x0FU, data_mode);
    lcd_nibble(value & 0x0FU, data_mode);
}

static void lcd_command(unsigned int value)
{
    lcd_byte(value, 0U);
    if (value == 0x01U) lcd_delay(300000UL); /* Clear-display settle. */
}

static void lcd_text(const char *message)
{
    while (*message != '\0') {
        lcd_byte((unsigned char)*message, 1U);
        ++message;
    }
}

int main(void)
{
    unsigned int pin;

    SystemCoreClockUpdate();

    /* Select GPIO for P0.23-P0.28 without changing other pin functions. */
    for (pin = 23U; pin <= 28U; ++pin) {
        LPC_PINCON->PINSEL1 &= ~(3UL << ((pin - 16U) * 2U));
    }
    LPC_GPIO0->FIODIR |= LCD_PINS;
    LPC_GPIO0->FIOCLR = LCD_PINS;

    lcd_delay(2000000UL); /* Power-up settle before Lab 9 sequence. */
    lcd_nibble(0x03U, 0U);
    lcd_nibble(0x03U, 0U);
    lcd_nibble(0x03U, 0U);
    lcd_nibble(0x02U, 0U); /* Enter four-bit mode. */
    lcd_command(0x28U);   /* Two lines, 5x8 font. */
    lcd_command(0x0CU);   /* Display on, cursor off. */
    lcd_command(0x06U);   /* Increment address. */
    lcd_command(0x01U);   /* Clear. */

    lcd_command(0x80U);   /* First line. */
    lcd_text("LCD TEST");
    lcd_command(0xC0U);   /* Second line. */
    lcd_text("BOOT OK");

    while (1) {
        /* Leave both messages visible. */
    }
}
