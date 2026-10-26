#ifndef FLICK_FLY_MPU6050_H
#define FLICK_FLY_MPU6050_H

/* Seven-bit addresses only; AD0 wiring decides which address is valid. */
#define MPU6050_ADDRESS_AD0_LOW  0x68U
#define MPU6050_ADDRESS_AD0_HIGH 0x69U
#define MPU6050_OK               0
#define MPU6050_BAD_ARGUMENT    -1
#define MPU6050_BUS_ERROR       -2
#define MPU6050_WRONG_ID        -3
#define MPU6050_BUS_NACK        -4
#define MPU6050_BUS_TIMEOUT     -5

struct mpu6050_raw {
    int ax, ay, az;
    int gx, gy, gz;
};

/* Bus functions must use finite timeouts and report NACK/timeouts distinctly.
 * Return zero only when the entire transaction succeeds; use the NACK and
 * TIMEOUT codes above, or BUS_ERROR for another transport failure.
 */
struct mpu6050_bus {
    int (*read)(unsigned char address, unsigned char first_register,
                unsigned char *bytes, unsigned int count);
    int (*write)(unsigned char address, unsigned char reg, unsigned char value);
};

int mpu6050_identify(const struct mpu6050_bus *bus, unsigned char address,
                     unsigned char *who_am_i);
int mpu6050_wake(const struct mpu6050_bus *bus, unsigned char address);
int mpu6050_read_raw(const struct mpu6050_bus *bus, unsigned char address,
                     struct mpu6050_raw *out);
#endif
