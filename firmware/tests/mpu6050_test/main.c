/* Flick & Fly: standalone LPC1768 / MPU6050 I2C1 test.
 *
 * Build ONLY this application C file, plus Keil startup_LPC17xx.s and
 * system_LPC17xx.c. No code in mpu6050.c or platform_unconfigured.c is used.
 *
 * Documented trainer routes: CNC 5=P0.19/SDA1, CNC 6=P0.20/SCL1,
 * CNC 10=ground (ALS-SDA-ARMCTXM3-01-UM1 p25). UART0 P0.2/P0.3 reaches
 * RS-232 CN9 through MAX3232 (trainer manual pp28-29); terminal 9600 8N1.
 * User states MPU wiring is connected according to the project manual.
 *
 * Assumptions needing physical review: exact breakout VCC/logic levels and
 * pull-up rail/value, AD0 bias, actual CNC cable orientation, any CNC1/U16
 * sharing, actual 12 MHz trainer oscillator, CN9 terminal cable/switch state.
 * I2C is deliberately set to standard-mode 100 kHz; the fitted bus has not
 * been measured. Do not interpret a build as a physical pass.
 *
 * Status on CN9: NACK, TIMEOUT, BUS_ERROR, WRONG_ID, or IDENTIFIED.
 * The MPU address is probed at both chip-documented 7-bit possibilities,
 * 0x68 and 0x69. WHO_AM_I must be 0x68. On success, ACCEL_CONFIG=0
 * selects +/-2 g and PWR_MGMT_1=0 wakes the sensor. Raw signed ax/ay/az
 * are printed repeatedly. No TFT pins are touched.
 */
#include <LPC17xx.h>

#define I2C_TIMEOUT_LOOPS 2000000UL
#define UART_TIMEOUT_LOOPS 2000000UL
#define I2C_BUS_HZ        100000UL
#define UART_BAUD         9600UL
#define I2_AA             0x04UL
#define I2_SI             0x08UL
#define I2_STO            0x10UL
#define I2_STA            0x20UL
#define I2_EN             0x40UL

enum test_result {
    TEST_OK = 0, TEST_BUS_ERROR = -2, TEST_WRONG_ID = -3,
    TEST_NACK = -4, TEST_TIMEOUT = -5
};

volatile int mpu_last_result = TEST_BUS_ERROR;
volatile unsigned int mpu_last_i2c_state = 0U;
volatile unsigned int mpu_address = 0U;
volatile unsigned int mpu_who_am_i = 0U;
volatile int mpu_ax = 0, mpu_ay = 0, mpu_az = 0;
volatile unsigned long mpu_samples = 0UL;
volatile unsigned long mpu_errors = 0UL;
volatile unsigned int uart_timeout = 0U;

static unsigned long peripheral_clock(unsigned int selector)
{
    /* UM10360 PCLKSEL encoding: 00=/4, 01=/1, 10=/2, 11=/8. */
    switch (selector & 3U) {
    case 1U: return SystemCoreClock;
    case 2U: return SystemCoreClock / 2UL;
    case 3U: return SystemCoreClock / 8UL;
    default: return SystemCoreClock / 4UL;
    }
}

static void uart_init(void)
{
    unsigned long pclk, divisor;
    LPC_SC->PCONP |= (1UL << 3);            /* UART0 power. */
    LPC_SC->PCLKSEL0 &= ~(3UL << 6);        /* CCLK/4. */
    LPC_PINCON->PINSEL0 =
        (LPC_PINCON->PINSEL0 & ~(15UL << 4)) | (5UL << 4);
    pclk = peripheral_clock((LPC_SC->PCLKSEL0 >> 6) & 3U);
    divisor = (pclk + (8UL * UART_BAUD)) / (16UL * UART_BAUD);
    if (divisor == 0UL) divisor = 1UL;
    if (divisor > 65535UL) divisor = 65535UL;
    LPC_UART0->LCR = 0x83U;                /* 8N1, divisor latch. */
    LPC_UART0->DLL = (unsigned char)(divisor & 0xFFUL);
    LPC_UART0->DLM = (unsigned char)((divisor >> 8) & 0xFFUL);
    LPC_UART0->FDR = 0x10U;                /* Integer divider only. */
    LPC_UART0->LCR = 0x03U;
    LPC_UART0->FCR = 0x07U;
    LPC_UART0->TER = 0x80U;
}

static void uart_char(char ch)
{
    unsigned long left = UART_TIMEOUT_LOOPS;
    while ((LPC_UART0->LSR & 0x20U) == 0U && left != 0UL) --left;
    if (left == 0UL) { uart_timeout = 1U; return; }
    LPC_UART0->THR = (unsigned char)ch;
}

static void uart_text(const char *s)
{
    while (*s != '\0') uart_char(*s++);
}

static void uart_unsigned(unsigned long value)
{
    char digits[10];
    unsigned int n = 0U;
    do {
        digits[n++] = (char)('0' + (value % 10UL));
        value /= 10UL;
    } while (value != 0UL && n < 10U);
    while (n != 0U) uart_char(digits[--n]);
}

static void uart_signed(int value)
{
    long signed_value = (long)value;
    if (signed_value < 0L) {
        uart_char('-');
        uart_unsigned((unsigned long)(-signed_value));
    } else {
        uart_unsigned((unsigned long)signed_value);
    }
}

static void uart_hex_byte(unsigned int value)
{
    const char hex[] = "0123456789ABCDEF";
    uart_char(hex[(value >> 4) & 15U]);
    uart_char(hex[value & 15U]);
}

static int i2c_stop(void)
{
    unsigned long left = I2C_TIMEOUT_LOOPS;
    LPC_I2C1->I2CONSET = I2_STO;
    LPC_I2C1->I2CONCLR = I2_SI | I2_STA | I2_AA;
    while ((LPC_I2C1->I2CONSET & I2_STO) != 0UL && left != 0UL) --left;
    return left == 0UL ? TEST_TIMEOUT : TEST_OK;
}

static int i2c_fail(int reason)
{
    int stop_result = i2c_stop();
    /* Return controller to an idle master state, without driving GPIO. */
    LPC_I2C1->I2CONCLR = I2_EN | I2_STA | I2_SI | I2_AA;
    LPC_I2C1->I2CONSET = I2_EN;
    return stop_result == TEST_OK ? reason : TEST_TIMEOUT;
}

static int i2c_wait(unsigned int expected)
{
    unsigned long left = I2C_TIMEOUT_LOOPS;
    unsigned int state;
    while ((LPC_I2C1->I2CONSET & I2_SI) == 0UL && left != 0UL) --left;
    if (left == 0UL) return TEST_TIMEOUT;
    state = (unsigned int)(LPC_I2C1->I2STAT & 0xF8UL);
    mpu_last_i2c_state = state;
    if (state == expected) return TEST_OK;
    if (state == 0x20U || state == 0x30U || state == 0x48U)
        return TEST_NACK;
    return TEST_BUS_ERROR;
}

static int i2c_start(unsigned int repeated)
{
    int result;
    LPC_I2C1->I2CONSET = I2_STA;
    LPC_I2C1->I2CONCLR = I2_SI;
    result = i2c_wait(repeated ? 0x10U : 0x08U);
    return result;
}

static int i2c_send(unsigned int byte, unsigned int expected)
{
    LPC_I2C1->I2DAT = byte & 0xFFU;
    LPC_I2C1->I2CONCLR = I2_STA | I2_SI;
    return i2c_wait(expected);
}

static int i2c_read_registers(unsigned int address, unsigned int reg,
                              unsigned char *data, unsigned int count)
{
    unsigned int i;
    int result;
    if (count == 0U || data == 0) return TEST_BUS_ERROR;
    result = i2c_start(0U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_send(address << 1, 0x18U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_send(reg, 0x28U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_start(1U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_send((address << 1) | 1U, 0x40U);
    if (result != TEST_OK) return i2c_fail(result);
    for (i = 0U; i < count; ++i) {
        if (i + 1U < count) LPC_I2C1->I2CONSET = I2_AA;
        else LPC_I2C1->I2CONCLR = I2_AA;
        LPC_I2C1->I2CONCLR = I2_SI;
        result = i2c_wait(i + 1U < count ? 0x50U : 0x58U);
        if (result != TEST_OK) return i2c_fail(result);
        data[i] = (unsigned char)LPC_I2C1->I2DAT;
    }
    result = i2c_stop();
    return result == TEST_OK ? TEST_OK : i2c_fail(result);
}

static int i2c_write_register(unsigned int address, unsigned int reg,
                              unsigned int value)
{
    int result = i2c_start(0U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_send(address << 1, 0x18U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_send(reg, 0x28U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_send(value, 0x28U);
    if (result != TEST_OK) return i2c_fail(result);
    result = i2c_stop();
    return result == TEST_OK ? TEST_OK : i2c_fail(result);
}

static void i2c_init(void)
{
    unsigned long pclk, half_period;
    LPC_SC->PCONP |= (1UL << 19);           /* I2C1 power. */
    LPC_SC->PCLKSEL1 &= ~(3UL << 6);        /* CCLK/4. */
    /* P0.19/P0.20 function 3 = SDA1/SCL1, UM10360 PINSEL1. */
    LPC_PINCON->PINSEL1 =
        (LPC_PINCON->PINSEL1 & ~(15UL << 6)) | (15UL << 6);
    /* Neither internal pull-up nor pull-down; external reviewed pull-ups
     * are required by the I2C bus. These values do not select a supply rail. */
    LPC_PINCON->PINMODE1 =
        (LPC_PINCON->PINMODE1 & ~(15UL << 6)) | (10UL << 6);
    LPC_PINCON->PINMODE_OD0 |= (1UL << 19) | (1UL << 20);
    pclk = peripheral_clock((LPC_SC->PCLKSEL1 >> 6) & 3U);
    half_period = pclk / (2UL * I2C_BUS_HZ);
    if (half_period < 4UL) half_period = 4UL;
    LPC_I2C1->I2CONCLR = I2_EN | I2_STA | I2_SI | I2_AA;
    LPC_I2C1->I2SCLH = half_period;
    LPC_I2C1->I2SCLL = half_period;
    LPC_I2C1->I2CONSET = I2_EN;
}

static int signed_word(unsigned char high, unsigned char low)
{
    unsigned int word = ((unsigned int)high << 8) | (unsigned int)low;
    return word < 0x8000U ? (int)word : (int)word - 65536;
}

static void report_error(int result)
{
    mpu_last_result = result;
    if (result == TEST_NACK) uart_text("NACK");
    else if (result == TEST_TIMEOUT) uart_text("TIMEOUT");
    else if (result == TEST_WRONG_ID) uart_text("WRONG_ID");
    else uart_text("BUS_ERROR");
    uart_text(" state=0x");
    uart_hex_byte(mpu_last_i2c_state);
    uart_text("\r\n");
}

static void pause_between_samples(void)
{
    volatile unsigned long n;
    /* Uncalibrated pacing only; no sample rate is claimed. */
    for (n = 0UL; n < 1000000UL; ++n) { }
}

int main(void)
{
    unsigned int candidates[2] = {0x68U, 0x69U};
    unsigned int index, chosen = 0U, bad_id = 0U;
    unsigned char identity = 0U, raw[6];
    int result = TEST_NACK;

    /* Keil's startup calls SystemInit before main. */
    SystemCoreClockUpdate();
    uart_init();
    uart_text("MPU6050 I2C1 single-file test; 100kHz assumed\r\n");
    i2c_init();

    for (index = 0U; index < 2U; ++index) {
        result = i2c_read_registers(candidates[index], 0x75U, &identity, 1U);
        if (result == TEST_OK) {
            mpu_who_am_i = identity;
            if (identity == 0x68U) {
                chosen = candidates[index];
                break;
            }
            bad_id = 1U;
        } else if (result != TEST_NACK) {
            report_error(result);
            while (1) { }
        }
    }
    if (chosen == 0U) {
        if (bad_id != 0U) {
            uart_text("WRONG_ID observed=0x");
            uart_hex_byte(mpu_who_am_i);
            uart_text("\r\n");
            mpu_last_result = TEST_WRONG_ID;
        } else report_error(TEST_NACK);
        while (1) { }
    }

    mpu_address = chosen;
    mpu_last_result = TEST_OK;
    uart_text("IDENTIFIED addr=0x");
    uart_hex_byte(chosen);
    uart_text(" who_am_i=0x");
    uart_hex_byte(identity);
    uart_text("\r\n");

    result = i2c_write_register(chosen, 0x6BU, 0x00U); /* Wake. */
    if (result == TEST_OK)
        result = i2c_write_register(chosen, 0x1CU, 0x00U); /* +/-2 g. */
    if (result != TEST_OK) {
        report_error(result);
        while (1) { }
    }

    while (1) {
        result = i2c_read_registers(chosen, 0x3BU, raw, 6U);
        if (result == TEST_OK) {
            mpu_ax = signed_word(raw[0], raw[1]);
            mpu_ay = signed_word(raw[2], raw[3]);
            mpu_az = signed_word(raw[4], raw[5]);
            mpu_last_result = TEST_OK;
            ++mpu_samples;
            uart_text("sample="); uart_unsigned(mpu_samples);
            uart_text(" ax="); uart_signed(mpu_ax);
            uart_text(" ay="); uart_signed(mpu_ay);
            uart_text(" az="); uart_signed(mpu_az);
            uart_text("\r\n");
        } else {
            ++mpu_errors;
            report_error(result);
        }
        pause_between_samples();
    }
}
