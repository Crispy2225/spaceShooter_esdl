#include <LPC17xx.h>
#include "tft_port.h"

/* Pattern contract for a future identified 128x160 controller driver.
 * RGB565 colors: black, red, green, blue, white. Coordinates are pixels.
 * No controller initialization commands are guessed here.
 */
int main(void)
{
    /* The Keil LPC17xx startup calls SystemInit before main. */
    SystemCoreClockUpdate();
    if (tft_port_init() != 0) while (1) { }
    if (tft_fill(0x0000U) != 0) while (1) { }
    if (tft_text(8U, 16U, "FLICK & FLY", 0xFFFFU) != 0) while (1) { }
    if (tft_mark(0U, 0U, 0xF800U) != 0) while (1) { }
    if (tft_mark(127U, 0U, 0x07E0U) != 0) while (1) { }
    if (tft_mark(0U, 159U, 0x001FU) != 0) while (1) { }
    if (tft_mark(127U, 159U, 0xFFFFU) != 0) while (1) { }
    while (1) { }
}
