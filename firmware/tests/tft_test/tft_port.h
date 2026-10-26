#ifndef FLICK_FLY_TFT_PORT_H
#define FLICK_FLY_TFT_PORT_H

/* The photographed 128x160 SPI module has no confirmed controller.
 * Every operation must return nonzero until a controller-specific driver,
 * reset/control routing, electrical plan and SPI configuration are reviewed.
 */
int tft_port_init(void);
int tft_fill(unsigned int rgb565);
int tft_text(unsigned int x, unsigned int y, const char *text,
             unsigned int rgb565);
int tft_mark(unsigned int x, unsigned int y, unsigned int rgb565);
#endif
