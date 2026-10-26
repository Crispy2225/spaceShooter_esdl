#include "mpu6050.h"

/* TDK InvenSense RM-MPU-6000A-00 rev 4.2: WHO_AM_I, PWR_MGMT_1,
 * ACCEL_XOUT_H through GYRO_ZOUT_L (temperature occupies bytes 6-7).
 */
#define REG_ACCEL_XOUT_H 0x3BU
#define REG_PWR_MGMT_1   0x6BU
#define REG_WHO_AM_I     0x75U

static int valid_address(unsigned char address)
{
    return address == MPU6050_ADDRESS_AD0_LOW ||
           address == MPU6050_ADDRESS_AD0_HIGH;
}

static int signed_word(unsigned char high, unsigned char low)
{
    unsigned int word = ((unsigned int)high << 8) | (unsigned int)low;
    return word < 0x8000U ? (int)word : (int)word - 65536;
}

static int transport_error(int result)
{
    if (result == MPU6050_BUS_NACK || result == MPU6050_BUS_TIMEOUT)
        return result;
    return MPU6050_BUS_ERROR;
}

int mpu6050_identify(const struct mpu6050_bus *bus, unsigned char address,
                     unsigned char *who_am_i)
{
    unsigned char value;
    int result;
    if (bus == 0 || bus->read == 0 || who_am_i == 0 || !valid_address(address))
        return MPU6050_BAD_ARGUMENT;
    result = bus->read(address, REG_WHO_AM_I, &value, 1U);
    if (result != 0)
        return transport_error(result);
    *who_am_i = value;
    return value == 0x68U ? MPU6050_OK : MPU6050_WRONG_ID;
}

int mpu6050_wake(const struct mpu6050_bus *bus, unsigned char address)
{
    int result;
    if (bus == 0 || bus->write == 0 || !valid_address(address))
        return MPU6050_BAD_ARGUMENT;
    /* Clear SLEEP and explicitly select the internal clock for this test. */
    result = bus->write(address, REG_PWR_MGMT_1, 0x00U);
    return result == 0 ? MPU6050_OK : transport_error(result);
}

int mpu6050_read_raw(const struct mpu6050_bus *bus, unsigned char address,
                     struct mpu6050_raw *out)
{
    unsigned char bytes[14];
    struct mpu6050_raw sample;
    int result;
    if (bus == 0 || bus->read == 0 || out == 0 || !valid_address(address))
        return MPU6050_BAD_ARGUMENT;
    result = bus->read(address, REG_ACCEL_XOUT_H, bytes, 14U);
    if (result != 0)
        return transport_error(result);
    sample.ax = signed_word(bytes[0], bytes[1]);
    sample.ay = signed_word(bytes[2], bytes[3]);
    sample.az = signed_word(bytes[4], bytes[5]);
    sample.gx = signed_word(bytes[8], bytes[9]);
    sample.gy = signed_word(bytes[10], bytes[11]);
    sample.gz = signed_word(bytes[12], bytes[13]);
    *out = sample; /* Failed transfers never overwrite the caller's sample. */
    return MPU6050_OK;
}
