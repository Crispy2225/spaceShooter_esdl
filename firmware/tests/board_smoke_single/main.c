/* Flick & Fly single-C-file LPC1768 board smoke test.
 *
 * Lab 7 and the trainer connector table place LEDs 1-8 on GPIO0 P0.4-P0.11
 * through CNA -> CNA1. The lab example uses FIOSET for LED on and FIOCLR
 * for LED off; actual polarity and ribbon routing still need observation.
 * No MPU6050 or TFT pins are configured by this program.
 *
 * Build with the installed LPC17xx header, CMSIS include files,
 * startup_LPC17xx.s and system_LPC17xx.c outside this source folder.
 */
#include <LPC17xx.h>

static void visible_delay(void)
{
    volatile unsigned int count;

    /* Software-loop pacing, as in Lab 7. Duration is not calibrated. */
    for (count = 0U; count < 1000000U; ++count) {
    }
}

int main(void)
{
    unsigned long led;
    unsigned int position;

    /* The installed startup calls SystemInit before entering main. */
    SystemCoreClockUpdate();

    LPC_PINCON->PINSEL0 &= 0xFF0000FFUL; /* P0.4-P0.11: GPIO. */
    LPC_GPIO0->FIODIR |= 0x00000FF0UL;  /* Eight outputs. */
    LPC_GPIO0->FIOCLR = 0x00000FF0UL;   /* Start with all eight off. */

    while (1) {
        led = 0x00000010UL;
        for (position = 0U; position < 8U; ++position) {
            LPC_GPIO0->FIOSET = led;
            visible_delay();
            led <<= 1;
        }

        led = 0x00000010UL;
        for (position = 0U; position < 8U; ++position) {
            LPC_GPIO0->FIOCLR = led;
            visible_delay();
            led <<= 1;
        }
    }
}
