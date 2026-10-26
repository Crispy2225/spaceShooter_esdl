#include "platform.h"

/* Deliberately non-operational: no UART route, I2C timing, pull-up rail,
 * module supply or AD0 bias has been approved for the photographed breakout.
 * This file permits source compilation but cannot produce a physical pass.
 */
int platform_init(void) { return -1; }
unsigned char platform_mpu_address(void) { return 0U; }
int platform_i2c_read(unsigned char a, unsigned char r,
                      unsigned char *b, unsigned int n)
{ (void)a; (void)r; (void)b; (void)n; return -1; }
int platform_i2c_write(unsigned char a, unsigned char r, unsigned char v)
{ (void)a; (void)r; (void)v; return -1; }
void platform_diag_text(const char *t) { (void)t; }
void platform_diag_unsigned(unsigned long v) { (void)v; }
void platform_diag_signed(int v) { (void)v; }
unsigned long platform_timestamp_ms(void) { return 0UL; }
void platform_sample_pause(void) { }
