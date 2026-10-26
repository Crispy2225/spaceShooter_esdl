/* Flick & Fly board-only test: MIT ES Lab Manual 2026, Labs 6-7.
 * Target: the documented ALS LPC1768 trainer, after board identity review.
 * CNA -> CNA1 ribbon and its orientation must be checked in the lab.
 */
#include <LPC17xx.h>

static void visible_delay(void)
{
    volatile unsigned int count;
    /* Lab 7 uses a simple software delay. Its duration is not calibrated. */
    for (count = 0; count < 1000000U; ++count) {
        /* Keep the loop visible to the compiler. */
    }
}

int main(void)
{
    unsigned long led;
    unsigned int position;

    /* Keil's installed startup_LPC17xx.s calls SystemInit before main.
     * Lab 7 calls it in main; avoid initializing the clocks twice.
     */
    SystemCoreClockUpdate();

    /* Lab 7: P0.4-P0.11 are CNA GPIO outputs for the trainer LED block. */
    LPC_PINCON->PINSEL0 &= 0xFF0000FFUL;
    LPC_GPIO0->FIODIR |= 0x00000FF0UL;
    LPC_GPIO0->FIOCLR = 0x00000FF0UL;

    while (1) {
        led = 0x00000010UL;
        for (position = 0; position < 8U; ++position) {
            LPC_GPIO0->FIOSET = led;
            visible_delay();
            led <<= 1;
        }
        led = 0x00000010UL;
        for (position = 0; position < 8U; ++position) {
            LPC_GPIO0->FIOCLR = led;
            visible_delay();
            led <<= 1;
        }
    }
}
