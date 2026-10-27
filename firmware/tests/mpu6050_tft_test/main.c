/* LPC1768 MPU6050 + Robu 1.8-inch ST7735S TFT diagnostic.
 *
 * MPU: copied from the owner's known-working I2C2 implementation in
 * C:\Users\Laptop\Downloads\main.c (P0.10 SDA2, P0.11 SCL2).
 *
 * TFT wiring required by THIS image (CNC contacts, trainer manual p25):
 *   TFT SCK/CLK -> CNC 1 P0.15/SCK (legacy SPI, PINSEL function 3)
 *   TFT CS      -> CNC 2 P0.16/GPIO (software chip select)
 *   TFT SDA/DIN -> CNC 4 P0.18/MOSI (legacy SPI, function 3)
 *   TFT GND     -> CNC 10 ground
 *   TFT RESET   -> CNC 7 P0.21/GPIO  [PROPOSED, NOT VERIFIED AS WIRED]
 *   TFT A0/DC   -> CNC 8 P0.22/GPIO  [PROPOSED, NOT VERIFIED AS WIRED]
 * VCC and LED need a separately verified supply/backlight plan; no MCU
 * GPIO in this image powers them. Do not flash assuming these wires exist.
 * CNC-to-CNC1 ribbon/jumper occupancy must be checked before sharing SPI.
 *
 * ST7735S identity/RGB565: Robu 1.8-inch manual (2025-02), pp1-2.
 * Init register values: Waveshare WSLCD1in8 LCD_Driver.cpp, LCD_InitReg(),
 * LCD_Reset(), LCD_Init(). Portrait MADCTL=0xC8 follows the ST7735S
 * 4D Systems 4DLCD-144 reference. Display RAM origin offsets
 * (1,2) follow that example and may need adjustment on this Robu module.
 */
#include <LPC17xx.h>
#include <stdio.h>

#define TFT_CS      (1UL << 16)
#define TFT_RST     (1UL << 21)
#define TFT_DC      (1UL << 22)
#define TFT_GPIO    (TFT_CS | TFT_RST | TFT_DC)
#define TFT_X_OFFSET 1U
#define TFT_Y_OFFSET 2U
#define BLACK       0x0000U
#define WHITE       0xFFFFU
#define CYAN        0x07FFU
#define RED         0xF800U

static volatile unsigned long ms_ticks;
static unsigned char accel_data[6];

void SysTick_Handler(void) { ++ms_ticks; }

static void delay_ms(unsigned long ms)
{
    unsigned long start = ms_ticks;
    while ((unsigned long)(ms_ticks - start) < ms) { }
}

/* Retain the reference's wake-up delay count. */
static void delay_lcd(unsigned int count)
{
    volatile unsigned int n;
    for (n = 0; n < count; ++n) { }
}

static int spi_byte(unsigned char value)
{
    unsigned long timeout = 3000000UL;
    LPC_SPI->SPDR = value;
    while ((LPC_SPI->SPSR & 0x80U) == 0U) {
        if (--timeout == 0UL) return 0;
    }
    (void)LPC_SPI->SPDR;
    return 1;
}

static int tft_command(unsigned char command)
{
    int ok;
    LPC_GPIO0->FIOCLR = TFT_DC | TFT_CS;
    ok = spi_byte(command);
    LPC_GPIO0->FIOSET = TFT_CS;
    return ok;
}

static int tft_data(unsigned char data)
{
    int ok;
    LPC_GPIO0->FIOSET = TFT_DC;
    LPC_GPIO0->FIOCLR = TFT_CS;
    ok = spi_byte(data);
    LPC_GPIO0->FIOSET = TFT_CS;
    return ok;
}

static int tft_reg(unsigned char command, const unsigned char *data,
                   unsigned int count)
{
    unsigned int i;
    if (!tft_command(command)) return 0;
    for (i = 0U; i < count; ++i)
        if (!tft_data(data[i])) return 0;
    return 1;
}

static int tft_spi_init(void)
{
    /* Legacy SPI PCLK = CCLK/4, preserving the I2C2 PCLKSEL1 setup. */
    LPC_SC->PCONP |= (1UL << 8);
    LPC_SC->PCLKSEL0 &= ~(3UL << 16);

    /* P0.15=SCK and P0.18=MOSI; CS, RESET, DC stay GPIO. */
    LPC_PINCON->PINSEL0 &= ~(3UL << 30);
    LPC_PINCON->PINSEL0 |=  (3UL << 30);
    LPC_PINCON->PINSEL1 &= ~((3UL << 0) | (3UL << 4) |
                             (3UL << 10) | (3UL << 12));
    LPC_PINCON->PINSEL1 |= (3UL << 4);
    LPC_GPIO0->FIODIR |= TFT_GPIO;
    LPC_GPIO0->FIOSET = TFT_GPIO;

    /* PCLK/32 = 0.75 MHz with the installed 96 MHz system clock. */
    LPC_SPI->SPCR = 0x20U;  /* Master, mode 0, MSB first, 8 bits. */
    LPC_SPI->SPCCR = 32U;  /* Even divider >= 8. */
    LPC_SPI->SPINT = 1U;
    return 1;
}

static int tft_init(void)
{
    static const unsigned char b1[] = {0x01,0x2C,0x2D};
    static const unsigned char b3[] = {0x01,0x2C,0x2D,0x01,0x2C,0x2D};
    static const unsigned char c0[] = {0xA2,0x02,0x84};
    static const unsigned char c2[] = {0x0A,0x00};
    static const unsigned char c3[] = {0x8A,0x2A};
    static const unsigned char c4[] = {0x8A,0xEE};
    static const unsigned char e0[] = {0x0F,0x1A,0x0F,0x18,
                                      0x2F,0x28,0x20,0x22,
                                      0x1F,0x1B,0x23,0x37,
                                      0x00,0x07,0x02,0x10};
    static const unsigned char e1[] = {0x0F,0x1B,0x0F,0x17,
                                      0x33,0x2C,0x29,0x2E,
                                      0x30,0x30,0x39,0x3F,
                                      0x00,0x07,0x03,0x10};
    static const unsigned char b4[] = {0x07};
    static const unsigned char c1[] = {0xC5};
    static const unsigned char c5[] = {0x0E};
    static const unsigned char f0[] = {0x01};
    static const unsigned char f6[] = {0x00};
    static const unsigned char colmod[] = {0x05};
    static const unsigned char madctl[] = {0xC8};

    if (!tft_spi_init()) return 0;
    LPC_GPIO0->FIOSET = TFT_RST;
    delay_ms(100UL);
    LPC_GPIO0->FIOCLR = TFT_RST;
    delay_ms(100UL);
    LPC_GPIO0->FIOSET = TFT_RST;
    delay_ms(100UL);

    if (!tft_reg(0xB1,b1,3U) || !tft_reg(0xB2,b1,3U) ||
        !tft_reg(0xB3,b3,6U) || !tft_reg(0xB4,b4,1U) ||
        !tft_reg(0xC0,c0,3U) || !tft_reg(0xC1,c1,1U) ||
        !tft_reg(0xC2,c2,2U) || !tft_reg(0xC3,c3,2U) ||
        !tft_reg(0xC4,c4,2U) || !tft_reg(0xC5,c5,1U) ||
        !tft_reg(0xE0,e0,16U) || !tft_reg(0xE1,e1,16U) ||
        !tft_reg(0xF0,f0,1U) || !tft_reg(0xF6,f6,1U) ||
        !tft_reg(0x3A,colmod,1U) || !tft_reg(0x36,madctl,1U) ||
        !tft_command(0x11U)) return 0;
    delay_ms(120UL);
    if (!tft_command(0x29U)) return 0;
    delay_ms(10UL);
    return 1;
}

static int tft_window(unsigned int x, unsigned int y,
                      unsigned int width, unsigned int height)
{
    unsigned int x0, x1, y0, y1;
    unsigned char p[4];
    if (width == 0U || height == 0U || x + width > 128U ||
        y + height > 160U) return 0;
    x0 = x + TFT_X_OFFSET;
    x1 = x0 + width - 1U;
    y0 = y + TFT_Y_OFFSET;
    y1 = y0 + height - 1U;
    p[0] = (unsigned char)(x0 >> 8); p[1] = (unsigned char)x0;
    p[2] = (unsigned char)(x1 >> 8); p[3] = (unsigned char)x1;
    if (!tft_reg(0x2AU,p,4U)) return 0;
    p[0] = (unsigned char)(y0 >> 8); p[1] = (unsigned char)y0;
    p[2] = (unsigned char)(y1 >> 8); p[3] = (unsigned char)y1;
    if (!tft_reg(0x2BU,p,4U)) return 0;
    return tft_command(0x2CU);
}

static int tft_fill_rect(unsigned int x, unsigned int y,
                         unsigned int width, unsigned int height,
                         unsigned int color)
{
    unsigned long n, pixels = (unsigned long)width * height;
    if (!tft_window(x,y,width,height)) return 0;
    for (n = 0UL; n < pixels; ++n)
        if (!tft_data((unsigned char)(color >> 8)) ||
            !tft_data((unsigned char)color)) return 0;
    return 1;
}

/* Compact 5x7 glyphs used by this diagnostic; bytes are vertical columns. */
static const unsigned char digits[10][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}
};
static const unsigned char letters[26][5] = {
    {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
    {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
};

static unsigned char glyph_column(char ch, unsigned int col)
{
    if (col >= 5U) return 0U;
    if (ch >= '0' && ch <= '9') return digits[ch - '0'][col];
    if (ch >= 'A' && ch <= 'Z') return letters[ch - 'A'][col];
    if (ch == '-') return col == 2U ? 0x08U : (col == 1U || col == 3U ? 0x08U : 0U);
    if (ch == '+') return col == 2U ? 0x1CU : (col == 1U || col == 3U ? 0x08U : 0U);
    if (ch == ':') return col == 2U ? 0x24U : 0U;
    return 0U;
}

static int tft_char(unsigned int x, unsigned int y, char ch,
                    unsigned int color)
{
    unsigned int row, col, pixel;
    if (!tft_window(x,y,6U,8U)) return 0;
    for (row = 0U; row < 8U; ++row) {
        for (col = 0U; col < 6U; ++col) {
            pixel = (glyph_column(ch,col) & (1U << row)) ? color : BLACK;
            if (!tft_data((unsigned char)(pixel >> 8)) ||
                !tft_data((unsigned char)pixel)) return 0;
        }
    }
    return 1;
}

static int tft_text(unsigned int x, unsigned int y, const char *s,
                    unsigned int color)
{
    while (*s != '\0' && x + 6U <= 128U) {
        if (!tft_char(x,y,*s++,color)) return 0;
        x += 6U;
    }
    return 1;
}

/* The following I2C2 transaction functions preserve the working reference. */
static void i2c2_init(void)
{
    LPC_SC->PCONP |= (1UL << 26);
    LPC_SC->PCLKSEL1 &= ~(3UL << 20);
    LPC_PINCON->PINSEL0 &= ~((3UL << 20) | (3UL << 22));
    LPC_PINCON->PINSEL0 |=  ((2UL << 20) | (2UL << 22));
    LPC_PINCON->PINMODE0 &= ~((3UL << 20) | (3UL << 22));
    LPC_PINCON->PINMODE0 |=  ((2UL << 20) | (2UL << 22));
    LPC_PINCON->PINMODE_OD0 |= (1UL << 10) | (1UL << 11);
    LPC_I2C2->I2SCLH = 125;
    LPC_I2C2->I2SCLL = 125;
    LPC_I2C2->I2CONCLR = 0x6C;
    LPC_I2C2->I2CONSET = 0x40;
}

static int i2c_wait(void)
{
    unsigned long timeout = 3000000UL;
    while (!(LPC_I2C2->I2CONSET & 0x08)) {
        if (--timeout == 0UL) return 0;
    }
    return 1;
}

static void i2c_stop(void)
{
    LPC_I2C2->I2CONSET = 0x10;
    LPC_I2C2->I2CONCLR = 0x28;
}

static int i2c_write_reg(unsigned char reg, unsigned char value)
{
    LPC_I2C2->I2CONSET = 0x20;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x08) goto error;
    LPC_I2C2->I2DAT = 0xD0;
    LPC_I2C2->I2CONCLR = 0x28;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x18) goto error;
    LPC_I2C2->I2DAT = reg;
    LPC_I2C2->I2CONCLR = 0x08;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x28) goto error;
    LPC_I2C2->I2DAT = value;
    LPC_I2C2->I2CONCLR = 0x08;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x28) goto error;
    i2c_stop();
    return 1;
error:
    i2c_stop();
    return 0;
}

static int i2c_read_accel(void)
{
    unsigned int n;
    LPC_I2C2->I2CONSET = 0x20;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x08) goto error;
    LPC_I2C2->I2DAT = 0xD0;
    LPC_I2C2->I2CONCLR = 0x28;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x18) goto error;
    LPC_I2C2->I2DAT = 0x3B;
    LPC_I2C2->I2CONCLR = 0x08;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x28) goto error;
    LPC_I2C2->I2CONSET = 0x20;
    LPC_I2C2->I2CONCLR = 0x08;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x10) goto error;
    LPC_I2C2->I2DAT = 0xD1;
    LPC_I2C2->I2CONCLR = 0x28;
    if (!i2c_wait() || LPC_I2C2->I2STAT != 0x40) goto error;
    for (n = 0U; n < 6U; ++n) {
        if (n < 5U) LPC_I2C2->I2CONSET = 0x04;
        else        LPC_I2C2->I2CONCLR = 0x04;
        LPC_I2C2->I2CONCLR = 0x08;
        if (!i2c_wait()) goto error;
        if (n < 5U && LPC_I2C2->I2STAT != 0x50) goto error;
        if (n == 5U && LPC_I2C2->I2STAT != 0x58) goto error;
        accel_data[n] = LPC_I2C2->I2DAT;
    }
    i2c_stop();
    return 1;
error:
    i2c_stop();
    return 0;
}

int main(void)
{
    short reading[3];
    char field[12];
    unsigned int axis;
    /* Vendor startup calls SystemInit before entering main. */
    SystemCoreClockUpdate();
    if (SysTick_Config(SystemCoreClock / 1000UL) != 0U) while (1) { }
    if (!tft_init()) while (1) { } /* Display path cannot report own failure. */
    if (!tft_fill_rect(0U,0U,128U,160U,BLACK) ||
        !tft_text(8U,8U,"MPU6050 TFT TEST",CYAN) ||
        !tft_text(8U,40U,"X:",WHITE) ||
        !tft_text(8U,64U,"Y:",WHITE) ||
        !tft_text(8U,88U,"Z:",WHITE)) while (1) { }

    i2c2_init();
    if (!i2c_write_reg(0x6BU,0x00U)) {
        (void)tft_text(8U,120U,"MPU WAKE ERROR",RED);
        while (1) { }
    }
    delay_lcd(1000000U);
    if (!i2c_write_reg(0x1AU,0x04U) ||
        !i2c_write_reg(0x19U,0x09U) ||
        !i2c_write_reg(0x1CU,0x00U)) {
        (void)tft_text(8U,120U,"MPU SETUP ERROR",RED);
        while (1) { }
    }
    while (1) {
        if (!i2c_read_accel()) {
            (void)tft_text(8U,120U,"MPU READ ERROR",RED);
            delay_ms(200UL);
            continue;
        }
        (void)tft_fill_rect(8U,120U,108U,8U,BLACK);
        reading[0] = (short)(((unsigned int)accel_data[0] << 8) | accel_data[1]);
        reading[1] = (short)(((unsigned int)accel_data[2] << 8) | accel_data[3]);
        reading[2] = (short)(((unsigned int)accel_data[4] << 8) | accel_data[5]);
        for (axis = 0U; axis < 3U; ++axis) {
            sprintf(field,"%+6d  ",(int)reading[axis]);
            if (!tft_text(26U,40U + axis * 24U,field,WHITE))
                while (1) { }
        }
        delay_ms(200UL);
    }
}
