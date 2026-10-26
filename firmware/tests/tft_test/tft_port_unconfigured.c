#include "tft_port.h"

/* Compileable fail-closed port. This sends no SPI bytes and drives no pins. */
int tft_port_init(void) { return -1; }
int tft_fill(unsigned int color) { (void)color; return -1; }
int tft_text(unsigned int x, unsigned int y, const char *s,
             unsigned int color)
{ (void)x; (void)y; (void)s; (void)color; return -1; }
int tft_mark(unsigned int x, unsigned int y, unsigned int color)
{ (void)x; (void)y; (void)color; return -1; }
