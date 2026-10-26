#include <LPC17xx.h>
#include "mpu6050.h"
#include "platform.h"

static void stop(void)
{
    while (1) { }
}

static void report_sample(const struct mpu6050_raw *s)
{
    platform_diag_text("ax="); platform_diag_signed(s->ax);
    platform_diag_text(" ay="); platform_diag_signed(s->ay);
    platform_diag_text(" az="); platform_diag_signed(s->az);
    platform_diag_text(" gx="); platform_diag_signed(s->gx);
    platform_diag_text(" gy="); platform_diag_signed(s->gy);
    platform_diag_text(" gz="); platform_diag_signed(s->gz);
    platform_diag_text("\r\n");
}

int main(void)
{
    struct mpu6050_bus bus;
    struct mpu6050_raw sample;
    unsigned char address, identity;
    unsigned long successes = 0UL, errors = 0UL;
    int result;

    /* The Keil LPC17xx startup calls SystemInit before main. */
    SystemCoreClockUpdate();
    if (platform_init() != 0) stop(); /* No bus activity on unapproved port. */
    address = platform_mpu_address();
    bus.read = platform_i2c_read;
    bus.write = platform_i2c_write;
    result = mpu6050_identify(&bus, address, &identity);
    if (result != MPU6050_OK) {
        platform_diag_text("address_7bit=");
        platform_diag_unsigned(address);
        platform_diag_text(" identity_error="); platform_diag_signed(result);
        if (result == MPU6050_WRONG_ID) {
            platform_diag_text(" observed_who_am_i=");
            platform_diag_unsigned(identity);
        }
        platform_diag_text(" success_count=0 error_count=1\r\n");
        stop();
    }
    platform_diag_text("who_am_i="); platform_diag_unsigned(identity);
    platform_diag_text(" address_7bit="); platform_diag_unsigned(address);
    platform_diag_text("\r\n");
    result = mpu6050_wake(&bus, address);
    if (result != MPU6050_OK) {
        platform_diag_text("wake_error="); platform_diag_signed(result);
        platform_diag_text(" success_count=0 error_count=1\r\n");
        stop();
    }
    while (1) {
        result = mpu6050_read_raw(&bus, address, &sample);
        if (result == MPU6050_OK) {
            ++successes;
            platform_diag_text("timestamp_ms=");
            platform_diag_unsigned(platform_timestamp_ms());
            platform_diag_text(" ");
            report_sample(&sample);
        } else {
            ++errors;
            platform_diag_text("read_error="); platform_diag_signed(result);
            platform_diag_text("\r\n");
        }
        platform_diag_text("success_count=");
        platform_diag_unsigned(successes);
        platform_diag_text(" error_count=");
        platform_diag_unsigned(errors);
        platform_diag_text("\r\n");
        platform_sample_pause();
    }
}
