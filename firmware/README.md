# Flick & Fly firmware

**Current milestone:** The isolated [M5 restart and high-score firmware](tests/restart_highscore_m5/README.md) implements the game. Its [physical verification record](tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md) contains owner-reported SW2 restart, continued M4.1 gameplay/calibration, and best-score retention after board power off/on. The [M5 hardware signal map](../manual/hardware-connections.md#current-m5-signal-map-for-the-ieee-report) documents I²C2, TFT SPI/GPIO, and the reported CNB-to-CNB1 SW2 ribbon while marking unverified wire endpoints and electrical levels. The physically flashed M5 HEX remains unidentified, and retention through firmware reprogramming is unverified. See the [current-state and evidence index](../docs/CURRENT_STATE_AND_EVIDENCE_2026-10-29.md) for the milestone trail, hardware assumptions, and IEEE-report evidence gaps.

**Later isolated M7:** [Decision-tree M7](tests/decision_tree_m7/README.md) retains the M5 game while compiling the M6 three-class tree for neutral/direction decisions. Host tests and the ARMCC 5 build passed. The owner now reports flashing M7 and seeing gameplay that appears the same as M5; no programming log or measured performance was supplied. M5 remains unchanged.

## Historical pre-integration tests (26–27 October 2026)

These are three **separate** LPC1768 diagnostic programs for the 27 October
2026 lab. They are not a game or a hardware pass **on their own**. Review the historical assumptions in
`HARDWARE_ASSUMPTIONS.md` before using a test on the fitted hardware.

## Source and hardware basis

- `C:\Users\Laptop\OneDrive\Documents\MIT\5th Sem\ES\ICT 3144  ESD LAB MANUAL_2026.pdf`:
  PDF pp. 5-10 (Lab 1 uVision assembly workflow), pp. 25-27 (Lab 6 LPC1768
  embedded C project, HEX option and trainer programming procedure), pp.
  28-29 (Lab 7 GPIO LED C example), pp. 37-41 (Lab 9 parallel 16x2 LCD,
  **not** this SPI TFT), and pp. 63-65 (Appendix C connectors). The manual
  contains no applicable MPU6050 I2C or TFT SPI C driver.
- `manual/hardware-readiness.md`: board/module photographs and outstanding
  electrical and connector questions. Its CNC signal candidates do not
  authorize module power or connection.
- TDK InvenSense, *MPU-6000/MPU-6050 Register Map and Descriptions*,
  RM-MPU-6000A-00 rev. 4.2, sections 4.17–4.19/4.28/4.32:
  https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
  The protocol uses WHO_AM_I 0x75 (expected 0x68), PWR_MGMT_1 0x6B, and
  ACCEL_XOUT_H 0x3B through GYRO_ZOUT_L 0x48. A 14-byte read includes two
  temperature bytes, which the raw-six-axis test skips. AD0 selects the
  documented seven-bit address 0x68 or 0x69; the actual breakout bias is
  unknown. Chip-level documentation does not establish breakout voltages.

## Test A: board-only LEDs

`tests/board_smoke/main.c` is the only test with an established output path
in the supplied lab manual. It selects GPIO for P0.4-P0.11, sets the output
direction, then turns the eight documented CNA1 LEDs on sequentially and off
sequentially. The delay is intentionally approximate, as in Lab 7. Confirm
the actual trainer identity and the CNA-to-CNA1 ribbon orientation first.

### Create/build a separate Keil uVision4 project in the lab

1. Create a **new project in this test's folder** in uVision4, select vendor
   NXP and device LPC1768. No `.uvproj` is supplied tonight. Do not reuse
   either practice `.uvproj`: those are
   assembly/simulator exercises with HEX generation off.
2. Add `main.c` to Source Group 1. Add exactly one device startup component
   `startup_LPC17xx.s` from the installed Keil LPC17xx support. Following
   Lab 6, copy `C:\Keil\ARM\Startup\NXP\LPC17xx\system_LPC17xx.c` into
   the new project folder and add **that copy**. Do not edit the installed
   shared file. The lab must confirm that the selected startup is compatible
   with the actual LPC1768 and project memory/clock settings. The installed
   `startup_LPC17xx.s` already calls `SystemInit` before `main`; the test
   calls `SystemCoreClockUpdate` in `main` but does not initialize twice,
   despite the Lab 7 sample placing both calls there.
   Lab 6's phrase `system_LPC17xx.s` for the startup prompt is inconsistent
   with the installed `startup_LPC17xx.s` and `system_LPC17xx.c`; inspect
   the files selected by Keil so neither startup nor system source is added
   twice. The installed header and CMSIS include roots are
   `C:\Keil\ARM\INC\NXP\LPC17xx` and `C:\Keil\ARM\CMSIS\Include`.
3. In target options confirm the actual device, ARMCC toolchain, memory and
   startup configuration. On **Output**, select **Create HEX File** (Lab 6).
   Build the target and retain the complete build log and output identity.
4. Identify the actual lab programmer and cable. Lab 6 describes Flash Magic
   6.01, CN9, SW21 and JP3, but this is a reference procedure to check
   against the present trainer and lab equipment. Its stated 9600 baud and
   advanced-options 115200 baud conflict; have the lab operator resolve
   that before programming. Record the verified settings. Do not flash a
   binary from a simulator-configured practice project.
5. A successful build or programmer verification is **not** Test A's pass.
   Observe the repeatable LED sequence on the actual trainer after reset.

The supplied personal computer has `C:\Keil\UV4\Uv4.exe`, ARMCC 5.03 Build
24 tools and LPC17xx header/startup/system files. On 26 October, direct
ARMCC command-line compilation, startup assembly, link and HEX conversion
of this board source succeeded in a temporary directory. This verifies the
source/toolchain combination used for that command, **not** a saved Keil GUI
project, matched oscillator, programmer or working trainer.

## Test B: MPU6050 protocol and blocked target port

This section describes an early, unconfigured **I²C1 diagnostic proposal**.
Its CNC 5/6 candidates are not the current M5 MPU6050 route; the game uses
I²C2 on P0.10/P0.11 via the alternatives in the [M5 signal map](../manual/hardware-connections.md#current-m5-signal-map-for-the-ieee-report).

`tests/mpu6050_test/mpu6050.c` holds only device-register operations. It
checks a selected seven-bit address, reads WHO_AM_I, clears SLEEP, and reads
14 consecutive bytes to extract signed raw accel/gyro words. A failed bus
transaction returns an error and never overwrites a valid sample with zero.
`main.c` reports identity, success/error counts, a timestamp and six signed raw values
through `platform.h`. `platform_unconfigured.c` fails initialization and
performs **no I2C traffic**. It is a compileable safety stub, not a sensor
test. Do not flash it as if it could verify the MPU.
The bus contract distinguishes `MPU6050_BUS_NACK` (-4),
`MPU6050_BUS_TIMEOUT` (-5), and other `MPU6050_BUS_ERROR` (-2); the protocol
passes the first two through. The later real port must implement finite
timeouts and return those codes. No timeout is implemented by the stub.

Before replacing that stub with **one** lab-reviewed LPC1768 port, establish:

- the fitted chip and breakout circuit, VCC range/current, I/O levels, SCL/
  SDA pull-up values and rail, AD0 bias, INT/auxiliary-pin treatment;
- current CNC/CNC1 ribbon and jumper state, connector pin 1, labelled adapter
  continuity and the signed electrical connection plan;
- the selected seven-bit address (0x68 or 0x69), approved I2C1 signal route,
  timing from a verified clock, and finite software/hardware timeouts;
- a **verified** board diagnostic output (for example, a lab-confirmed UART
  route) and timebase that can print identity, counts, errors, timestamps
  and six signed values.

The trainer documentation identifies CNC 5 as P0.19/SDA1 and CNC 6 as
P0.20/SCL1 only as board-side candidates. No I2C clock value, module rail,
pull-up rail or diagnostic UART pins are assigned here. If the lab cannot
verify those items, leave the port stub in place and record Test B as blocked.
Build each test as its own uVision target with its own `main.c`; add the three
MPU `.c` files plus startup/system support. Never include the unconfigured
port together with a later real port.

## Test C: TFT structure, controller blocked

`tests/tft_test/main.c` defines the desired 128x160 pattern: black field,
white text, and colored corner marks. `tft_port.h` is the small rendering
contract. `tft_port_unconfigured.c` returns failure from every operation and
sends **no SPI command**. Its build cannot demonstrate the display.

Identify the actual controller and module schematic first. A future port
must implement its documented initialization and reset sequence, pixel/text
operations, reviewed SPI mode/clock and control-pin allocation, supply and
LED/backlight drive. CNC 1/2/4 are board-side SCK0/SSEL0/MOSI0 candidates
in the trainer documentation, not an approved TFT connection. The parallel
16x2 LCD code in Lab 9 cannot initialize this module. Build this standalone
test with its two `.c` files and LPC17xx startup/system support only after
the port is implemented and reviewed; the current stub is just a structure
and compile check.

## Evidence rule

Use `LAB_CHECKLIST.md` for the physical sequence. Keep build logs, exact
firmware identity, programmer settings, connection review, UART traces and
photos. Compilation, simulated output and browser illustrations are never
recorded as physical MPU/TFT passes. The full game was a later milestone
relative to these pre-integration tests. The proposed classifier comparison
remains unverified; see the current-state index.
