/* LPC1768 M5: M4.1 gameplay plus SW2 restart and flash best score.
 * SW2 is REPORTED to be CNB->CNB1, P2.12, active low. Its physical
 * continuity, pressed/released voltage and external pull circuit have NOT
 * been measured. Do not treat this source configuration as electrical proof.
 * The MPU6050/TFT 5 V module and I/O compatibility is also unverified.
 * Flash Magic preservation of score sectors has NOT been physically tested.
 * TFT and I2C2 code through i2c_read_accel is copied from the M3 baseline.
 * M3 tilt configuration and calibration are preserved below.
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

/* Provisional mapping: no measured left/neutral/right set is recorded.
 * Change these definitions after physical testing, then rebuild.
 */
#define MOVE_AXIS       0   /* 0=X, 1=Y, 2=Z */
#define MOVE_SIGN       1   /* +1: positive delta moves right; -1 reverses */
#define DEAD_ZONE       150L
#define COUNTS_PER_STEP 1500L
#define CAL_SAMPLES     32U
#define CAL_INTERVAL_MS 20UL
#define CAL_MAX_SPAN    600L
#define FRAME_MS        40UL
#define SHOT_PERIOD_MS  600UL
#define SHOT_STEP_PX    4U
#if (MOVE_AXIS < 0) || (MOVE_AXIS > 2)
#error MOVE_AXIS must be 0, 1 or 2
#endif
#if (MOVE_SIGN != 1) && (MOVE_SIGN != -1)
#error MOVE_SIGN must be +1 or -1
#endif
#if (DEAD_ZONE < 0L) || (DEAD_ZONE > 32767L)
#error DEAD_ZONE must be between 0 and 32767
#endif
#if (COUNTS_PER_STEP < 1L)
#error COUNTS_PER_STEP must be positive
#endif

static volatile unsigned long ms_ticks;
static unsigned char accel_data[6];

void SysTick_Handler(void) { ++ms_ticks; }

static void delay_ms(unsigned long ms)
{
    unsigned long start = ms_ticks;
    while ((unsigned long)(ms_ticks - start) < ms) { }
}

/* Preserve the combined test's sensor wake settling delay count. */
static void delay_lcd(unsigned int count)
{
    volatile unsigned int n;
    for (n = 0U; n < count; ++n) { }
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

#define SCREEN_W       128U
#define SCREEN_H       160U
#define GRID_TOP       32U
#define GRID_BOTTOM    144U
#define GRID_BG        0x0008U
#define GRID_LINE      0x4208U
#define GREEN          0x07E0U
#define YELLOW         0xFFE0U
#define SHIP_W         17U
#define SHIP_H         15U
#define SHIP_Y         108U
#define SHIP_LEFT      8U
#define SHIP_RIGHT     103U
#define SHOT_W         3U
#define SHOT_H         5U
#define SHOT_TOP       34U
#define SHOT_START     (SHIP_Y - SHOT_H - 1U)
#define ENEMY_W        11U
#define ENEMY_H        9U
#define ENEMY_START_Y  34U
#define ENEMY_SLOTS    3U
#define ENEMY_PERIOD_MS 900UL
#define ENEMY_STEP_MS  80UL
#define ENEMY_STEP_PX  2U
#define HIT_GRACE_MS   900UL
#define SCORE_CAP      99999UL
#define INITIAL_LIVES  3U
#define STAR_COLOR     0x8410U
#define COCKPIT_BLUE   0x001FU
#define CAL_RETRIES    3U
#define CAL_FAILED_MAX 8U
#define CAL_TIMEOUT_MS 2500UL
#define CAL_SETTLE_MS  2000UL
#define CAL_RETRY_MS   500UL
#define HIT_FLASH_MS   250UL
#define SW2_BIT        (1UL << 12)
#define SW2_DEBOUNCE_MS 40UL
#define SCORE_SECTOR_A 14U
#define SCORE_SECTOR_B 15U
#define SCORE_BASE_A   0x0000E000UL
#define SCORE_BASE_B   0x0000F000UL
#define SCORE_SECTOR_SIZE 4096U
#define SCORE_RECORD_SIZE 256U
#define SCORE_MAGIC    0x354D4646UL /* "FFM5" in little-endian flash */
#define SCORE_VERSION  1UL

typedef struct {
    unsigned int x, y;
    unsigned char active;
} Enemy;

typedef enum { PLAYING, GAME_OVER } GameState;

typedef struct {
    unsigned long score;
    unsigned long sequence;
    unsigned int sector;
    int valid;
} ScoreRecord;

static unsigned long score_write_words[SCORE_RECORD_SIZE / 4U];

static void sw2_init(void)
{
    /* GPIO function, internal pull-up, input; no EINT or GPIO interrupt. */
    LPC_PINCON->PINSEL4 &= ~(3UL << 24);
    LPC_PINCON->PINMODE4 &= ~(3UL << 24);
    LPC_PINCON->PINMODE_OD2 &= ~SW2_BIT;
    LPC_GPIO2->FIODIR &= ~SW2_BIT;
}

/* Call frequently with SysTick time. Return one edge after a stable release. */
static int sw2_press_edge(unsigned long now)
{
    static unsigned long changed_at;
    static unsigned int raw_pressed, stable_pressed, armed, initialized;
    unsigned int pressed = (LPC_GPIO2->FIOPIN & SW2_BIT) == 0U;
    if (!initialized) {
        initialized = 1U;
        raw_pressed = stable_pressed = pressed;
        changed_at = now;
        return 0;
    }
    if (pressed != raw_pressed) {
        raw_pressed = pressed;
        changed_at = now;
    }
    if (pressed != stable_pressed &&
        (unsigned long)(now - changed_at) >= SW2_DEBOUNCE_MS) {
        stable_pressed = pressed;
        if (!pressed) armed = 1U;
        else if (armed) {
            armed = 0U;
            return 1;
        }
    }
    if (!stable_pressed && !armed &&
        (unsigned long)(now - changed_at) >= SW2_DEBOUNCE_MS)
        armed = 1U;
    return 0;
}

static unsigned long read_le32(const unsigned char *p)
{
    return (unsigned long)p[0] | ((unsigned long)p[1] << 8) |
           ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);
}

static void write_le32(unsigned char *p, unsigned long value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
    p[2] = (unsigned char)(value >> 16);
    p[3] = (unsigned char)(value >> 24);
}

static unsigned long score_crc(const unsigned char *p)
{
    unsigned long crc = 0xFFFFFFFFUL;
    unsigned int i, bit;
    for (i = 0U; i < 20U; ++i) {
        crc ^= p[i];
        for (bit = 0U; bit < 8U; ++bit)
            crc = (crc >> 1) ^ ((crc & 1UL) ? 0xEDB88320UL : 0UL);
    }
    return ~crc;
}

static int score_valid(const unsigned char *p, unsigned long *score,
                       unsigned long *sequence)
{
    unsigned long value = read_le32(p + 12U);
    unsigned long order = read_le32(p + 8U);
    if (read_le32(p) != SCORE_MAGIC || read_le32(p + 4U) != SCORE_VERSION ||
        value == 0UL || value > SCORE_CAP || order == 0UL || order > value ||
        read_le32(p + 16U) != ~value ||
        read_le32(p + 20U) != score_crc(p)) return 0;
    *score = value;
    *sequence = order;
    return 1;
}

static int score_blank(const unsigned char *p)
{
    unsigned int i;
    for (i = 0U; i < SCORE_RECORD_SIZE; ++i)
        if (p[i] != 0xFFU) return 0;
    return 1;
}

static unsigned long score_sector_base(unsigned int sector)
{
    return sector == SCORE_SECTOR_A ? SCORE_BASE_A : SCORE_BASE_B;
}

static ScoreRecord score_scan(void)
{
    ScoreRecord best = {0UL, 0UL, SCORE_SECTOR_A, 0};
    unsigned int sector, slot;
    unsigned long value, order;
    for (sector = SCORE_SECTOR_A; sector <= SCORE_SECTOR_B; ++sector) {
        for (slot = 0U; slot < SCORE_SECTOR_SIZE / SCORE_RECORD_SIZE; ++slot) {
            const unsigned char *p = (const unsigned char *)
                (score_sector_base(sector) + slot * SCORE_RECORD_SIZE);
            if (score_valid(p, &value, &order) &&
                (!best.valid || value > best.score ||
                 (value == best.score && order > best.sequence))) {
                best.score = value;
                best.sequence = order;
                best.sector = sector;
                best.valid = 1;
            }
        }
    }
    return best;
}

static int score_first_blank(unsigned int sector)
{
    unsigned int slot;
    for (slot = 0U; slot < SCORE_SECTOR_SIZE / SCORE_RECORD_SIZE; ++slot) {
        const unsigned char *p = (const unsigned char *)
            (score_sector_base(sector) + slot * SCORE_RECORD_SIZE);
        if (score_blank(p)) return (int)slot;
    }
    return -1;
}

static int score_sector_blank(unsigned int sector)
{
    unsigned int slot;
    for (slot = 0U; slot < SCORE_SECTOR_SIZE / SCORE_RECORD_SIZE; ++slot) {
        const unsigned char *p = (const unsigned char *)
            (score_sector_base(sector) + slot * SCORE_RECORD_SIZE);
        if (!score_blank(p)) return 0;
    }
    return 1;
}

static int score_iap(unsigned long code, unsigned long sector,
                     unsigned long destination)
{
    unsigned long command[5] = {0UL,0UL,0UL,0UL,0UL};
    unsigned long result[5] = {0UL,0UL,0UL,0UL,0UL};
    unsigned long primask = __get_PRIMASK();
    void (*entry)(unsigned long[], unsigned long[]) =
        (void (*)(unsigned long[], unsigned long[]))0x1FFF1FF1UL;
    command[0] = code;
    if (code == 50UL || code == 52UL) {
        command[1] = sector;
        command[2] = sector;
        if (code == 52UL) command[3] = SystemCoreClock / 1000UL;
    } else if (code == 51UL) {
        command[1] = destination;
        command[2] = (unsigned long)score_write_words;
        command[3] = SCORE_RECORD_SIZE;
        command[4] = SystemCoreClock / 1000UL;
    } else return 0;
    __disable_irq(); /* ROM IAP does not mask SysTick; flash is busy. */
    entry(command, result);
    __set_PRIMASK(primask);
    return result[0] == 0UL;
}

static int score_save(unsigned long new_score, unsigned long previous)
{
    ScoreRecord best = score_scan();
    unsigned int target;
    int slot;
    unsigned long sequence, destination, read_score, read_sequence;
    unsigned char *p = (unsigned char *)score_write_words;
    unsigned int i;
    if (new_score <= previous || new_score > SCORE_CAP ||
        (best.valid && new_score <= best.score)) return 0;
    sequence = best.valid ? best.sequence + 1UL : 1UL;
    if (sequence > new_score) return 0;
    target = best.valid ? best.sector : SCORE_SECTOR_A;
    slot = score_first_blank(target);
    if (slot < 0) {
        target = target == SCORE_SECTOR_A ? SCORE_SECTOR_B : SCORE_SECTOR_A;
        /* The highest valid record is in the other sector, or none exists. */
        if (best.valid && target == best.sector) return 0;
        if (!score_iap(50UL, target, 0UL) ||
            !score_iap(52UL, target, 0UL) ||
            !score_sector_blank(target))
            return 0;
        slot = 0;
    }
    destination = score_sector_base(target) +
                  (unsigned long)slot * SCORE_RECORD_SIZE;
    for (i = 0U; i < SCORE_RECORD_SIZE; ++i) p[i] = 0xFFU;
    write_le32(p, SCORE_MAGIC);
    write_le32(p + 4U, SCORE_VERSION);
    write_le32(p + 8U, sequence);
    write_le32(p + 12U, new_score);
    write_le32(p + 16U, ~new_score);
    write_le32(p + 20U, score_crc(p));
    if (!score_iap(50UL, target, 0UL) ||
        !score_iap(51UL, target, destination)) return 0;
    if (!score_valid((const unsigned char *)destination,
                     &read_score, &read_sequence) ||
        read_score != new_score || read_sequence != sequence) return 0;
    for (i = 0U; i < SCORE_RECORD_SIZE; ++i)
        if (((const unsigned char *)destination)[i] != p[i]) return 0;
    return 1;
}

static const unsigned long ship_rows[SHIP_H] = {
    0x00100UL, 0x00380UL, 0x007C0UL, 0x00FE0UL, 0x01FF0UL,
    0x03FF8UL, 0x07FFCUL, 0x0FFFFUL, 0x1FFFFUL, 0x1FFFFUL,
    0x0FFFFUL, 0x0638CUL, 0x04104UL, 0x08002UL, 0x10001UL
};

static const unsigned int enemy_rows[ENEMY_H] = {
    0x020U, 0x0F8U, 0x1FCU, 0x3FEU, 0x7FFU,
    0x7FFU, 0x36DU, 0x202U, 0x401U
};

static unsigned int grid_pixel(unsigned int x, unsigned int y)
{
    if ((x % 16U) == 0U || (y % 16U) == 0U) return GRID_LINE;
    if (((x * 37U + y * 19U) & 1023U) == 0U) return STAR_COLOR;
    return GRID_BG;
}

static int tft_color(unsigned int color)
{
    return tft_data((unsigned char)(color >> 8)) &&
           tft_data((unsigned char)color);
}

/* M3 ship sprite and mathematical background restoration, unchanged. */
static int draw_ship_window(unsigned int x, int show_ship)
{
    unsigned int row, col, color;
    if (x < SHIP_LEFT || x > SHIP_RIGHT ||
        SHIP_Y + SHIP_H > GRID_BOTTOM) return 0;
    if (!tft_window(x, SHIP_Y, SHIP_W, SHIP_H)) return 0;
    for (row = 0U; row < SHIP_H; ++row) {
        for (col = 0U; col < SHIP_W; ++col) {
            color = grid_pixel(x + col, SHIP_Y + row);
            if (show_ship &&
                (ship_rows[row] & (1UL << (SHIP_W - 1U - col))) != 0UL) {
                if (row < 4U) color = CYAN;
                else if (row < 10U) color = WHITE;
                else if (row < 13U) color = RED;
                else color = YELLOW;
                if ((row == 6U || row == 7U) &&
                    (col == 7U || col == 8U || col == 9U))
                    color = COCKPIT_BLUE;
            }
            if (!tft_color(color)) return 0;
        }
    }
    return 1;
}

/* M3 projectile appearance and erase behavior, unchanged. */
static int draw_shot_window(unsigned int x, unsigned int y, int show_shot)
{
    unsigned int row, col, color;
    if (x + SHOT_W > SCREEN_W || y < GRID_TOP ||
        y + SHOT_H > SHIP_Y) return 0;
    if (!tft_window(x, y, SHOT_W, SHOT_H)) return 0;
    for (row = 0U; row < SHOT_H; ++row) {
        for (col = 0U; col < SHOT_W; ++col) {
            color = show_shot ? YELLOW : grid_pixel(x + col, y + row);
            if (!tft_color(color)) return 0;
        }
    }
    return 1;
}

/* Eleven-column enemy mask; green hull and two red eyes. */
static int draw_enemy_window(unsigned int x, unsigned int y, int show_enemy)
{
    unsigned int row, col, color;
    if (x + ENEMY_W > SCREEN_W || y < GRID_TOP ||
        y + ENEMY_H > GRID_BOTTOM) return 0;
    if (!tft_window(x, y, ENEMY_W, ENEMY_H)) return 0;
    for (row = 0U; row < ENEMY_H; ++row) {
        for (col = 0U; col < ENEMY_W; ++col) {
            color = grid_pixel(x + col, y + row);
            if (show_enemy &&
                (enemy_rows[row] & (1U << (ENEMY_W - 1U - col))) != 0U) {
                color = GREEN;
                if (row == 4U && (col == 3U || col == 7U)) color = RED;
            }
            if (!tft_color(color)) return 0;
        }
    }
    return 1;
}

static int draw_scene(void)
{
    unsigned int x, y;
    if (!tft_fill_rect(0U, 0U, SCREEN_W, SCREEN_H, BLACK) ||
        !tft_fill_rect(0U, GRID_TOP, SCREEN_W,
                       GRID_BOTTOM - GRID_TOP, GRID_BG)) return 0;
    for (x = 0U; x < SCREEN_W; x += 16U)
        if (!tft_fill_rect(x, GRID_TOP, 1U,
                           GRID_BOTTOM - GRID_TOP, GRID_LINE)) return 0;
    for (y = GRID_TOP; y < GRID_BOTTOM; y += 16U)
        if (!tft_fill_rect(0U, y, SCREEN_W, 1U, GRID_LINE)) return 0;
    for (y = GRID_TOP; y < GRID_BOTTOM; ++y)
        for (x = 0U; x < SCREEN_W; ++x)
            if (grid_pixel(x, y) == STAR_COLOR &&
                !tft_fill_rect(x, y, 1U, 1U, STAR_COLOR)) return 0;
    return 1;
}

static int status(const char *message, unsigned int color)
{
    return tft_fill_rect(4U, 148U, 120U, 8U, BLACK) &&
           tft_text(4U, 148U, message, color);
}

static void display_failure(void)
{
    while (1) { }
}

static void decode_accel(short raw[3])
{
    raw[0] = (short)(((unsigned int)accel_data[0] << 8) | accel_data[1]);
    raw[1] = (short)(((unsigned int)accel_data[2] << 8) | accel_data[3]);
    raw[2] = (short)(((unsigned int)accel_data[4] << 8) | accel_data[5]);
}

typedef enum {
    CAL_SETUP, CAL_SETTLE, CAL_COLLECT, CAL_RETRY, CAL_READY, CAL_FAILED
} CalState;

/* All attempts and reads are bounded. A failed read changes only the failure
 * count: it cannot contribute zero or partial bytes to the neutral sums.
 */
static int calibrate(long neutral[3])
{
    CalState state = CAL_SETUP;
    short raw[3], minimum[3], maximum[3];
    long sum[3] = {0L, 0L, 0L};
    unsigned long entered_at = ms_ticks, last_sample = ms_ticks;
    unsigned long now;
    unsigned int attempt = 0U, valid = 0U, failed = 0U, axis;
    const char *reason = "CAL TIMEOUT";
    char progress[] = "KEEP STILL 00/32";

    while (state != CAL_READY && state != CAL_FAILED) {
        now = ms_ticks;
        switch (state) {
        case CAL_SETUP:
            ++attempt;
            if (!status("KEEP STILL", CYAN)) display_failure();
            i2c2_init();
            if (!i2c_write_reg(0x6BU, 0x00U)) {
                reason = "MPU WAKE ERROR";
                state = CAL_RETRY;
                entered_at = ms_ticks;
                if (!status(reason, RED)) display_failure();
                break;
            }
            delay_lcd(1000000U); /* Reference wake settling count. */
            if (!i2c_write_reg(0x1AU, 0x04U) ||
                !i2c_write_reg(0x19U, 0x09U) ||
                !i2c_write_reg(0x1CU, 0x00U)) {
                reason = "MPU SETUP ERROR";
                state = CAL_RETRY;
                entered_at = ms_ticks;
                if (!status(reason, RED)) display_failure();
                break;
            }
            state = CAL_SETTLE;
            entered_at = ms_ticks;
            break;
        case CAL_SETTLE:
            if ((unsigned long)(now - entered_at) >= CAL_SETTLE_MS) {
                valid = failed = 0U;
                for (axis = 0U; axis < 3U; ++axis) sum[axis] = 0L;
                state = CAL_COLLECT;
                entered_at = now;
                last_sample = now - CAL_INTERVAL_MS;
                if (!status(progress, CYAN)) display_failure();
            }
            break;
        case CAL_COLLECT:
            if ((unsigned long)(now - entered_at) >= CAL_TIMEOUT_MS) {
                reason = "CAL TIMEOUT";
                state = CAL_RETRY;
                entered_at = now;
                if (!status(reason, RED)) display_failure();
                break;
            }
            if ((unsigned long)(now - last_sample) < CAL_INTERVAL_MS) break;
            last_sample = now;
            if (!i2c_read_accel()) {
                ++failed;
                if (failed >= CAL_FAILED_MAX) {
                    reason = "CAL READ ERROR";
                    state = CAL_RETRY;
                    entered_at = ms_ticks;
                    if (!status(reason, RED)) display_failure();
                }
                break;
            }
            decode_accel(raw);
            for (axis = 0U; axis < 3U; ++axis) {
                if (valid == 0U || raw[axis] < minimum[axis])
                    minimum[axis] = raw[axis];
                if (valid == 0U || raw[axis] > maximum[axis])
                    maximum[axis] = raw[axis];
                sum[axis] += (long)raw[axis];
            }
            ++valid;
            if (valid % 8U == 0U) {
                progress[11] = (char)('0' + valid / 10U);
                progress[12] = (char)('0' + valid % 10U);
                if (!status(progress, CYAN)) display_failure();
            }
            if (valid == CAL_SAMPLES) {
                for (axis = 0U; axis < 3U; ++axis) {
                    if ((long)maximum[axis] - (long)minimum[axis] >
                        CAL_MAX_SPAN) break;
                }
                if (axis != 3U) {
                    reason = "CAL UNSTABLE";
                    state = CAL_RETRY;
                    entered_at = ms_ticks;
                    if (!status(reason, RED)) display_failure();
                } else {
                    for (axis = 0U; axis < 3U; ++axis)
                        neutral[axis] = sum[axis] / (long)CAL_SAMPLES;
                    state = CAL_READY;
                }
            }
            break;
        case CAL_RETRY:
            if (attempt >= CAL_RETRIES) {
                state = CAL_FAILED;
            } else if ((unsigned long)(now - entered_at) >= CAL_RETRY_MS) {
                state = CAL_SETUP;
            }
            break;
        default:
            state = CAL_FAILED;
            break;
        }
    }
    if (state == CAL_READY) {
        if (!status("READY", GREEN)) display_failure();
        return 1;
    }
    if (!status(reason, RED) ||
        !tft_fill_rect(0U, 16U, SCREEN_W, 13U, BLACK) ||
        !tft_text(4U, 16U, "RESET TO RETRY", RED)) display_failure();
    return 0;
}

static unsigned int next_ship_x(unsigned int current, long delta)
{
    long magnitude, step, next;
    if (delta >= -DEAD_ZONE && delta <= DEAD_ZONE) return current;
    magnitude = delta < 0L ? -delta : delta;
    step = 1L + (magnitude - DEAD_ZONE) / COUNTS_PER_STEP;
    if (step > 5L) step = 5L;
    next = (long)current + (delta < 0L ? -step : step);
    if (next < (long)SHIP_LEFT) next = (long)SHIP_LEFT;
    if (next > (long)SHIP_RIGHT) next = (long)SHIP_RIGHT;
    return (unsigned int)next;
}

static int overlap(unsigned int ax, unsigned int ay,
                   unsigned int aw, unsigned int ah,
                   unsigned int bx, unsigned int by,
                   unsigned int bw, unsigned int bh)
{
    return ax < bx + bw && bx < ax + aw &&
           ay < by + bh && by < ay + ah;
}

static void score_text(unsigned long score, char text[6])
{
    unsigned int i;
    text[5] = '\0';
    for (i = 5U; i > 0U; --i) {
        text[i - 1U] = (char)('0' + score % 10UL);
        score /= 10UL;
    }
}

static int update_hud(unsigned long score, unsigned int lives)
{
    char digits[6];
    char life_text[8] = "LIVES 0";
    score_text(score, digits);
    life_text[6] = (char)('0' + lives);
    return tft_text(40U, 3U, digits, YELLOW) &&
           tft_text(4U, 16U, life_text, CYAN);
}

static int draw_hud(unsigned long score, unsigned int lives)
{
    return tft_fill_rect(0U, 0U, SCREEN_W, GRID_TOP, BLACK) &&
           tft_text(4U, 3U, "SCORE", WHITE) &&
           update_hud(score, lives) &&
           tft_fill_rect(0U, 29U, SCREEN_W, 1U, COCKPIT_BLUE);
}

static int draw_game_over(unsigned long score, unsigned long best, int save_failed)
{
    char final_digits[6], best_digits[6];
    score_text(score, final_digits);
    score_text(best, best_digits);
    return tft_fill_rect(0U, 0U, SCREEN_W, SCREEN_H, BLACK) &&
           tft_fill_rect(12U, 27U, 104U, 1U, CYAN) &&
           tft_text(34U, 35U, "GAME OVER", RED) &&
           tft_text(22U, 57U, "FINAL SCORE", WHITE) &&
           tft_text(49U, 69U, final_digits, YELLOW) &&
           tft_text(28U, 88U, "BEST SCORE", WHITE) &&
           tft_text(49U, 100U, best_digits, YELLOW) &&
           tft_text(19U, 124U, "SW2 TO RESTART", CYAN) &&
           tft_fill_rect(12U, 139U, 104U, 1U, CYAN) &&
           (!save_failed || tft_text(25U, 148U, "SAVE FAILED", RED));
}

int main(void)
{
    static const unsigned int lanes[4] = {8U, 39U, 70U, 101U};
    Enemy enemies[ENEMY_SLOTS] = {{0U,0U,0U},{0U,0U,0U},{0U,0U,0U}};
    GameState state = PLAYING;
    long neutral[3], delta;
    short raw[3];
    unsigned int ship_x = (SHIP_LEFT + SHIP_RIGHT) / 2U;
    unsigned int next_x, shot_x = 0U, shot_y = 0U;
    unsigned int lives = INITIAL_LIVES, lane_index = 0U, i;
    unsigned long score = 0UL;
    unsigned long best_score;
    unsigned long last_frame, last_shot, last_enemy_spawn;
    unsigned long last_enemy_step, last_hit = 0UL;
    unsigned long last_hit_flash = 0UL;
    int shot_active = 0, sensor_error = 0, hit_grace = 0, hit_flash = 0;
    int hud_dirty, enemy_step, ship_changed;

    SystemCoreClockUpdate();
    if (SysTick_Config(SystemCoreClock / 1000UL) != 0U) display_failure();
    sw2_init();
    best_score = score_scan().score;
    (void)sw2_press_edge(ms_ticks);
    if (!tft_init() || !draw_scene() ||
        !draw_hud(score, lives) || !status("KEEP STILL", CYAN))
        display_failure();
    if (!calibrate(neutral)) while (1) { } /* Final error awaits reset. */
    if (!draw_ship_window(ship_x, 1)) display_failure();
    last_frame = last_shot = last_enemy_spawn = last_enemy_step = ms_ticks;

    while (1) {
        unsigned long now = ms_ticks;
        int restart_edge = sw2_press_edge(now);
        if (state == GAME_OVER) {
            if (restart_edge) {
                for (i = 0U; i < ENEMY_SLOTS; ++i) {
                    enemies[i].x = enemies[i].y = 0U;
                    enemies[i].active = 0U;
                }
                ship_x = (SHIP_LEFT + SHIP_RIGHT) / 2U;
                next_x = ship_x;
                shot_x = shot_y = 0U;
                shot_active = 0;
                lives = INITIAL_LIVES;
                lane_index = 0U;
                score = 0UL;
                sensor_error = hit_grace = hit_flash = 0;
                last_hit = last_hit_flash = 0UL;
                last_frame = last_shot = last_enemy_spawn =
                    last_enemy_step = ms_ticks;
                if (!draw_scene() || !draw_hud(score, lives) ||
                    !status("I2C OK", CYAN) ||
                    !draw_ship_window(ship_x, 1)) display_failure();
                state = PLAYING;
            }
            continue;
        }
        if ((unsigned long)(now - last_frame) < FRAME_MS) continue;
        last_frame = now;
        next_x = ship_x;
        hud_dirty = 0;
        if (i2c_read_accel()) {
            decode_accel(raw);
            delta = ((long)raw[MOVE_AXIS] - neutral[MOVE_AXIS]) * MOVE_SIGN;
            next_x = next_ship_x(ship_x, delta);
            if (sensor_error) {
                if (!status(hit_flash ? "HIT!" : "I2C OK",
                            hit_flash ? RED : CYAN)) display_failure();
                sensor_error = 0;
            }
        } else {
            /* Ignore partial data from a failed I2C transaction. */
            if (!sensor_error && !status("MPU READ ERROR", RED))
                display_failure();
            sensor_error = 1;
        }
        if (hit_flash && (unsigned long)(now - last_hit_flash) >=
                         HIT_FLASH_MS) {
            hit_flash = 0;
            if (!sensor_error && !status("I2C OK", CYAN))
                display_failure();
        }

        /* Restore every old dynamic enemy/shot window before composing the
         * new frame. Overlap is safe because no entity is drawn until all
         * old windows have been restored from the same grid function.
         */
        if (shot_active && !draw_shot_window(shot_x, shot_y, 0))
            display_failure();
        for (i = 0U; i < ENEMY_SLOTS; ++i)
            if (enemies[i].active &&
                !draw_enemy_window(enemies[i].x, enemies[i].y, 0))
                display_failure();
        ship_changed = next_x != ship_x;
        if (ship_changed && !draw_ship_window(ship_x, 0))
            display_failure();

        enemy_step = (unsigned long)(now - last_enemy_step) >= ENEMY_STEP_MS;
        if (enemy_step) last_enemy_step = now;
        for (i = 0U; i < ENEMY_SLOTS; ++i) {
            if (!enemies[i].active || !enemy_step) continue;
            enemies[i].y += ENEMY_STEP_PX;
            if (enemies[i].y + ENEMY_H >= GRID_BOTTOM) {
                enemies[i].active = 0U;
                if (lives > 0U) --lives;
                hud_dirty = 1;
            }
        }
        if (lives == 0U) {
            state = GAME_OVER;
            {
                int save_failed = 0;
                if (score > best_score) {
                    if (score_save(score, best_score)) best_score = score;
                    else save_failed = 1;
                }
                if (!draw_game_over(score, best_score, save_failed))
                    display_failure();
            }
            continue;
        }
        if (shot_active) {
            if (shot_y <= SHOT_TOP + SHOT_STEP_PX)
                shot_active = 0;
            else
                shot_y -= SHOT_STEP_PX;
        }
        if ((unsigned long)(now - last_enemy_spawn) >= ENEMY_PERIOD_MS) {
            last_enemy_spawn = now;
            for (i = 0U; i < ENEMY_SLOTS; ++i) {
                if (!enemies[i].active) {
                    enemies[i].x = lanes[lane_index];
                    enemies[i].y = ENEMY_START_Y;
                    enemies[i].active = 1U;
                    lane_index = (lane_index + 1U) % 4U;
                    break;
                }
            }
        }
        if (!shot_active &&
            (unsigned long)(now - last_shot) >= SHOT_PERIOD_MS) {
            shot_x = next_x + SHIP_W / 2U - SHOT_W / 2U;
            shot_y = SHOT_START;
            shot_active = 1;
            last_shot = now;
        }
        ship_x = next_x;

        /* One shot is consumed by the first intersecting enemy only. */
        if (shot_active) {
            for (i = 0U; i < ENEMY_SLOTS; ++i) {
                if (enemies[i].active &&
                    overlap(shot_x, shot_y, SHOT_W, SHOT_H,
                            enemies[i].x, enemies[i].y, ENEMY_W, ENEMY_H)) {
                    shot_active = 0;
                    enemies[i].active = 0U;
                    if (score < SCORE_CAP) ++score;
                    hud_dirty = 1;
                    break;
                }
            }
        }
        if (hit_grace && (unsigned long)(now - last_hit) >= HIT_GRACE_MS)
            hit_grace = 0;
        for (i = 0U; i < ENEMY_SLOTS; ++i) {
            if (enemies[i].active &&
                overlap(ship_x, SHIP_Y, SHIP_W, SHIP_H,
                        enemies[i].x, enemies[i].y, ENEMY_W, ENEMY_H)) {
                enemies[i].active = 0U;
                if (!hit_grace && lives > 0U) {
                    --lives;
                    last_hit = now;
                    hit_grace = 1;
                    hud_dirty = 1;
                    hit_flash = 1;
                    last_hit_flash = now;
                    if (!sensor_error && !status("HIT!", RED))
                        display_failure();
                }
            }
        }
        if (lives == 0U) {
            state = GAME_OVER;
            {
                int save_failed = 0;
                if (score > best_score) {
                    if (score_save(score, best_score)) best_score = score;
                    else save_failed = 1;
                }
                if (!draw_game_over(score, best_score, save_failed))
                    display_failure();
            }
            continue;
        }
        if (ship_changed && !draw_ship_window(ship_x, 1))
            display_failure();
        for (i = 0U; i < ENEMY_SLOTS; ++i)
            if (enemies[i].active &&
                !draw_enemy_window(enemies[i].x, enemies[i].y, 1))
                display_failure();
        if (shot_active && !draw_shot_window(shot_x, shot_y, 1))
            display_failure();
        if (hud_dirty && !update_hud(score, lives)) display_failure();
    }
}

