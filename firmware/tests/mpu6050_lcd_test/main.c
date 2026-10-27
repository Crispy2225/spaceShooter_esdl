/* LPC1768 trainer: MPU6050 I2C2 results on the verified 16x2 LCD.
 * One application C file, plus installed Keil startup/system build inputs.
 * LCD: CND->CNAD, P0.23..26=D4..D7, P0.27=RS, P0.28=EN (Lab 9).
 * Working reference: P0.10/SDA2 and P0.11/SCL2 (CNA7/8 or CN6 7/8).
 * Actual MPU wire endpoints remain unverified; the earlier CNC5/6 plan
 * selects a different bus. Supply, pull-ups, AD0 and cable need checking.
 * Axes are signed raw register counts, not calibrated physical units.
 */
#include <LPC17xx.h>

#define LCD_DATA 0x07800000UL
#define LCD_RS   0x08000000UL
#define LCD_EN   0x10000000UL
#define LCD_PINS (LCD_DATA | LCD_RS | LCD_EN)
#define WAIT_LIMIT 2000000UL
#define I2_AA 0x04UL
#define I2_SI 0x08UL
#define I2_STO 0x10UL
#define I2_STA 0x20UL
#define I2_EN 0x40UL

enum result { OK = 0, BUS_ERROR = -2, WRONG_ID = -3,
              NACK = -4, TIMEOUT = -5 };

static void lcd_delay(unsigned long count)
{
    volatile unsigned long i;
    for (i = 0UL; i < count; ++i) { }
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
    if (value == 0x01U) lcd_delay(300000UL);
}

static void lcd_init(void)
{
    unsigned int pin;
    for (pin = 23U; pin <= 28U; ++pin)
        LPC_PINCON->PINSEL1 &= ~(3UL << ((pin - 16U) * 2U));
    LPC_GPIO0->FIODIR |= LCD_PINS;
    LPC_GPIO0->FIOCLR = LCD_PINS;
    lcd_delay(2000000UL);
    lcd_nibble(0x03U, 0U);
    lcd_nibble(0x03U, 0U);
    lcd_nibble(0x03U, 0U);
    lcd_nibble(0x02U, 0U);
    lcd_command(0x28U);
    lcd_command(0x0CU);
    lcd_command(0x06U);
    lcd_command(0x01U);
}

static void lcd_line(unsigned int row, const char *text)
{
    unsigned int count = 0U;
    lcd_command(row == 0U ? 0x80U : 0xC0U);
    while (*text != '\0' && count < 16U) {
        lcd_byte((unsigned char)*text++, 1U);
        ++count;
    }
    while (count++ < 16U) lcd_byte(' ', 1U);
}

static void pause_one_second(void)
{
    unsigned long ticks = SystemCoreClock / 1000UL;
    unsigned int ms;
    /* Poll SysTick; no interrupt is enabled. Timing follows the installed
     * startup clock calculation and has not been measured on the board. */
    if (ticks == 0UL || ticks > 0x1000000UL) {
        lcd_delay(20000000UL);
        return;
    }
    SysTick->LOAD = ticks - 1UL;
    SysTick->VAL = 0UL;
    SysTick->CTRL = 0x05UL;
    for (ms = 0U; ms < 1000U; ++ms) {
        unsigned long left = WAIT_LIMIT;
        while ((SysTick->CTRL & 0x10000UL) == 0UL && left != 0UL) --left;
        if (left == 0UL) break;
    }
    SysTick->CTRL = 0UL;
}

static int i2c_stop(void)
{
    unsigned long left = WAIT_LIMIT;
    LPC_I2C2->I2CONSET = I2_STO;
    LPC_I2C2->I2CONCLR = I2_SI | I2_STA | I2_AA;
    while ((LPC_I2C2->I2CONSET & I2_STO) != 0UL && left != 0UL) --left;
    return left == 0UL ? TIMEOUT : OK;
}

static int i2c_fail(int reason)
{
    int stopped = i2c_stop();
    LPC_I2C2->I2CONCLR = I2_EN | I2_STA | I2_SI | I2_AA;
    LPC_I2C2->I2CONSET = I2_EN;
    return stopped == OK ? reason : TIMEOUT;
}

static int i2c_wait(unsigned int expected)
{
    unsigned long left = WAIT_LIMIT;
    unsigned int state;
    while ((LPC_I2C2->I2CONSET & I2_SI) == 0UL && left != 0UL) --left;
    if (left == 0UL) return TIMEOUT;
    state = (unsigned int)(LPC_I2C2->I2STAT & 0xF8UL);
    if (state == expected) return OK;
    if (state == 0x20U || state == 0x30U || state == 0x48U) return NACK;
    return BUS_ERROR;
}

static int i2c_start(unsigned int repeated)
{
    LPC_I2C2->I2CONSET = I2_STA;
    LPC_I2C2->I2CONCLR = I2_SI;
    return i2c_wait(repeated != 0U ? 0x10U : 0x08U);
}

static int i2c_send(unsigned int byte, unsigned int expected)
{
    LPC_I2C2->I2DAT = byte & 0xFFU;
    LPC_I2C2->I2CONCLR = I2_STA | I2_SI;
    return i2c_wait(expected);
}

static int i2c_read(unsigned int address, unsigned int reg,
                    unsigned char *data, unsigned int count)
{
    unsigned int i;
    int result;
    if (count == 0U || data == 0) return BUS_ERROR;
    result = i2c_start(0U);
    if (result != OK) return i2c_fail(result);
    result = i2c_send(address << 1, 0x18U);
    if (result != OK) return i2c_fail(result);
    result = i2c_send(reg, 0x28U);
    if (result != OK) return i2c_fail(result);
    result = i2c_start(1U);
    if (result != OK) return i2c_fail(result);
    result = i2c_send((address << 1) | 1U, 0x40U);
    if (result != OK) return i2c_fail(result);
    for (i = 0U; i < count; ++i) {
        if (i + 1U < count) LPC_I2C2->I2CONSET = I2_AA;
        else LPC_I2C2->I2CONCLR = I2_AA;
        LPC_I2C2->I2CONCLR = I2_SI;
        result = i2c_wait(i + 1U < count ? 0x50U : 0x58U);
        if (result != OK) return i2c_fail(result);
        data[i] = (unsigned char)LPC_I2C2->I2DAT;
    }
    result = i2c_stop();
    return result == OK ? OK : i2c_fail(result);
}

static int i2c_write(unsigned int address, unsigned int reg,
                     unsigned int value)
{
    int result = i2c_start(0U);
    if (result != OK) return i2c_fail(result);
    result = i2c_send(address << 1, 0x18U);
    if (result != OK) return i2c_fail(result);
    result = i2c_send(reg, 0x28U);
    if (result != OK) return i2c_fail(result);
    result = i2c_send(value, 0x28U);
    if (result != OK) return i2c_fail(result);
    result = i2c_stop();
    return result == OK ? OK : i2c_fail(result);
}

static void i2c_init(void)
{
    LPC_SC->PCONP |= (1UL << 26);
    LPC_SC->PCLKSEL1 &= ~(3UL << 20); /* I2C2 PCLK = CCLK/4. */
    LPC_PINCON->PINSEL0 =
        (LPC_PINCON->PINSEL0 & ~((3UL << 20) | (3UL << 22))) |
        ((2UL << 20) | (2UL << 22));
    /* External pull-ups are required; their rail is not set by firmware. */
    LPC_PINCON->PINMODE0 =
        (LPC_PINCON->PINMODE0 & ~((3UL << 20) | (3UL << 22))) |
        ((2UL << 20) | (2UL << 22));
    LPC_PINCON->PINMODE_OD0 |= (1UL << 10) | (1UL << 11);
    LPC_I2C2->I2CONCLR = I2_EN | I2_STA | I2_SI | I2_AA;
    LPC_I2C2->I2SCLH = 125U;
    LPC_I2C2->I2SCLL = 125U;
    LPC_I2C2->I2CONSET = I2_EN;
}

static int signed_word(unsigned char high, unsigned char low)
{
    unsigned int word = ((unsigned int)high << 8) | (unsigned int)low;
    return word < 0x8000U ? (int)word : (int)word - 65536;
}

static void axis_line(char axis, int value)
{
    char line[17], digits[6];
    unsigned int length = 0U, used = 0U;
    long number = (long)value;
    unsigned long magnitude;
    line[length++] = 'A'; line[length++] = axis; line[length++] = '=';
    if (number < 0L) { line[length++] = '-'; magnitude = (unsigned long)(-number); }
    else magnitude = (unsigned long)number;
    do {
        digits[used++] = (char)('0' + (magnitude % 10UL));
        magnitude /= 10UL;
    } while (magnitude != 0UL && used < 6U);
    while (used != 0U) line[length++] = digits[--used];
    line[length++] = ' '; line[length++] = 'R';
    line[length++] = 'A'; line[length++] = 'W';
    line[length] = '\0';
    lcd_line(1U, line);
}

static void show_error(int result)
{
    if (result == NACK) lcd_line(0U, "MPU: NOT FOUND");
    else if (result == TIMEOUT) lcd_line(0U, "I2C TIMEOUT");
    else if (result == WRONG_ID) lcd_line(0U, "WRONG WHOAMI");
    else lcd_line(0U, "I2C BUS ERROR");
    lcd_line(1U, "");
}

int main(void)
{
    const unsigned int candidates[2] = {0x68U, 0x69U};
    unsigned char identity = 0U, raw[6];
    unsigned int i, address = 0U, wrong_id = 0U, axis = 0U;
    int result = NACK, ax, ay, az;

    /* Installed startup calls SystemInit before main. */
    SystemCoreClockUpdate();
    lcd_init();
    lcd_line(0U, "MPU: PROBING");
    lcd_line(1U, "PLEASE WAIT");
    i2c_init();
    for (i = 0U; i < 2U; ++i) {
        result = i2c_read(candidates[i], 0x75U, &identity, 1U);
        if (result == OK) {
            if (identity == 0x68U) { address = candidates[i]; break; }
            wrong_id = 1U;
        } else if (result != NACK) {
            show_error(result);
            while (1) { }
        }
    }
    if (address == 0U) {
        show_error(wrong_id != 0U ? WRONG_ID : NACK);
        while (1) { }
    }
    result = i2c_write(address, 0x6BU, 0x00U); /* Wake. */
    if (result == OK) {
        lcd_delay(1000000UL); /* Match the working reference's wake delay. */
        result = i2c_write(address, 0x1AU, 0x04U); /* 21 Hz DLPF. */
    }
    if (result == OK) result = i2c_write(address, 0x19U, 0x09U);
    if (result == OK) result = i2c_write(address, 0x1CU, 0x00U); /* +/-2g. */
    if (result != OK) {
        show_error(result);
        while (1) { }
    }
    lcd_line(0U, "MPU: FOUND");
    while (1) {
        result = i2c_read(address, 0x3BU, raw, 6U);
        if (result == OK) {
            ax = signed_word(raw[0], raw[1]);
            ay = signed_word(raw[2], raw[3]);
            az = signed_word(raw[4], raw[5]);
            lcd_line(0U, "MPU: FOUND");
            if (axis == 0U) axis_line('X', ax);
            else if (axis == 1U) axis_line('Y', ay);
            else axis_line('Z', az);
            axis = (axis + 1U) % 3U;
        } else show_error(result);
        pause_one_second();
    }
}
