#ifndef FLICK_FLY_MPU_PLATFORM_H
#define FLICK_FLY_MPU_PLATFORM_H

/* LAB CONFIGURATION GATE. Replace the unconfigured implementation only after
 * board/connector/electrical review and an observed diagnostic output path.
 * I2C read must perform a register-pointer write then a complete read, with
 * finite timeouts, NACK detection and STOP/recovery on all error paths.
 * The implementation must return the MPU6050_BUS_NACK/TIMEOUT codes from
 * mpu6050.h, and platform_init must establish a tested diagnostic path.
 */
int platform_init(void);
unsigned char platform_mpu_address(void);
int platform_i2c_read(unsigned char address, unsigned char first_register,
                      unsigned char *bytes, unsigned int count);
int platform_i2c_write(unsigned char address, unsigned char reg,
                       unsigned char value);
void platform_diag_text(const char *text);
void platform_diag_unsigned(unsigned long value);
void platform_diag_signed(int value);
unsigned long platform_timestamp_ms(void);
void platform_sample_pause(void);
#endif
