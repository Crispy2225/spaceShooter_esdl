# 27 October 2026 hardware validation record

**Status:** Board smoke build verified. Programming and physical behavior not
verified. MPU6050 and TFT source builds verified only; both physical tests are
blocked. No hardware was connected, powered, erased or programmed in this
session.

## Tools and build evidence

- Keil µVision `C:\Keil\UV4\Uv4.exe`: file version 4.70.0.0.
- `C:\Keil\ARM\ARMCC\bin\{armcc,armasm,armlink,fromelf}.exe`:
  5.03 Build 24, MDK-ARM Lite (each tool's `--vsn` output).
- Flash Magic `C:\Program Files (x86)\Flash Magic\FlashMagic.exe`:
  file product version 13.65. Its GUI and programming connection were not
  operated or verified.

Each build uses the installed `LPC17xx.h`, CMSIS include root,
`startup_LPC17xx.s`, and `system_LPC17xx.c`. Commands, individual exit codes,
and output are in `build/2026-10-27/<target>/build.log`. The board source is
`tests/board_smoke/main.c`; MPU sources are `main.c`, `mpu6050.c`, and
`platform_unconfigured.c`; TFT sources are `main.c` and
`tft_port_unconfigured.c`. All final compile, assembly, link and FromELF
steps exited 0, without warnings or errors. The board build log preserves
two initial linker attempts: one failed with L6002U; the next linked with
L6319W but omitted the vector table. Neither output was accepted. The final
link uses `--no_remove --first=startup_LPC17xx.o(RESET)` with object names
resolved from the build directory, `--ro-base=0x00000000`,
`--rw-base=0x10000000`, `--cpu=Cortex-M3` and
`--entry=Reset_Handler`.

The final board AXF starts with the Cortex-M vector table at address 0.
FromELF's raw HEX left vector word 7 (offset 0x1C) zero, so a separate
`board_smoke_lpc1768.hex` was generated with that word set to the two's
complement of words 0–6 (`0xEFFFF486`). The first eight 32-bit vector words
then sum to zero modulo 2^32, and all 116 Intel HEX record checksums were
checked. The highest data address is `0x0000070B`; the linked RAM base is
`0x10000000`. This is a structural LPC1768 image check, not proof that the
fitted chip, oscillator or trainer are compatible. The reviewed candidate
image is:

`build/2026-10-27/board_smoke/board_smoke_lpc1768.hex`

SHA-256:
`41C4EFBE9CC112B84584D2FCB523D11E2DFF1E7965F346B76B77C2783D1C149D`.
The raw `board_smoke.hex` should not be selected for programming.

The MPU and TFT HEX files in their corresponding build directories are
**stub artifacts**. Their initialization routines return failure before
any peripheral transaction; they are not hardware-test firmware.

## Programming gate

Windows currently maps `COM9` and `COM8` to standard Bluetooth serial
links. `COM5` is a started CH340 USB-serial device, but its cable has not
been traced to trainer CN9 or the ISP circuit. Thus the screenshot's COM9
selection is not a verified programmer path. Flash Magic communication was
not attempted. There is no programming or verification log.

Lab manual PDF pp. 26–27 describes CN9 with a 9-pin cross cable, SW21 on,
JP3 shorted, LPC1768, 12 MHz oscillator, ISP interface, **Erase Blocks Used
by Hex File**, and **Verify After Programming**. It says 9600 baud in the
main Communications settings but 115200 in Advanced Communications. The
present screenshot's 57600 setting adds a third unverified value. The actual
trainer identity, cable route, ISP/reset handling, clock and baud choice
must be resolved with the operator before selecting the image or erase
option. Do not use an entire-device or mass-erase option. The user must
confirm the target board, image and erase action before pressing Start.

Expected board behavior from source: P0.4–P0.11 are cleared initially; each
of eight LEDs is set in ascending order, then each is cleared in ascending
order, repeatedly. The software delay is uncalibrated. The CNA–CNA1 ribbon,
polarity and actual LED behavior have not been observed.

## Peripheral gates

The candidate CNC contacts in `manual/hardware-readiness.md` are board-side
signals, not approved breadboard connections. MPU SDA/SCL/GND have candidate
CNC 5/6/10; VCC, pull-up rail/value, AD0 bias, auxiliary pins, INT,
diagnostic output, connector orientation and U16 sharing remain unverified.
`platform_unconfigured.c` has no pin mux, I²C timing, timeout-backed bus
driver, UART or timebase. Expected later trace: WHO_AM_I (chip-level default
0x68), chosen seven-bit address, timestamps, six signed raw values, and
success/error counts with NACK or timeout identified. No such trace exists.

TFT SCK/CS/SDA/GND have candidate CNC 1/2/4/10. The actual controller,
module supply and logic limits, RESET, A0, LED/backlight circuit, SPI mode,
clock and shared U5/U6 routing are unverified. `tft_port_unconfigured.c`
sends no SPI bytes. The source requests black fill, white `FLICK & FLY`
text at (8,16), red at (0,0), green at (127,0), blue at (0,159), and white
at (127,159), assuming a 128×160 orientation. No screen observation exists.

Before either module is connected or powered, complete the existing
connection and electrical review in `manual/hardware-readiness.md`, including
contact-1/keying and unpowered continuity, exact module documentation,
common ground, supply/current and logic levels, pull-ups or backlight drive,
and present ribbon/jumper state. A breadboard does not resolve these gates.

## Next operator action

With the trainer unpowered, provide a current readable photograph of the
fitted MCU and board markings, CN9-to-PC cable path, CNA/CNA1 ribbon, JP3,
SW21 and ISP/reset controls. Identify which Windows port belongs to that
cable (COM5 is a candidate, not a conclusion), and resolve the lab manual's
9600/115200 conflict with the actual Flash Magic 13.65 setup. Then review
the exact board HEX, erase-block setting and target with the user before any
programming attempt. After verified programming, record Flash Magic's log
and the actual LED observation separately.

## Single-file board-smoke follow-up

The separate source `tests/board_smoke_single/main.c` is the only C file in
its test folder. Its current SHA-256 is
`BC21D090F49928C7660B3B8E314DD84E94DF623B92F7A0243D528E8FE89D43BA`,
matching `build/2026-10-27/board_smoke_single/build.log`. The selected image
`build/2026-10-27/board_smoke_single/board_smoke_single_lpc1768.hex` still
has SHA-256
`41C4EFBE9CC112B84584D2FCB523D11E2DFF1E7965F346B76B77C2783D1C149D`,
matching that log. Its compile, assembly, link and HEX conversion steps
exited 0. The vector checksum and Intel HEX records were checked.

The user reports that the trainer manual's CNA-to-LED map was verified,
identifies COM5 as the programming port, and reports that the lab operator
confirmed **9600 baud**. These are user-supplied confirmations; no Flash
Magic connection or physical board response has been observed here.
Flash Magic preparation is LPC1768 (LPC1700), COM5, 9600 baud, the exact
single-file HEX above, **Erase Blocks Used by Hex File**, and **Verify After
Programming**. The actual trainer-specific ISP/reset steps are **BLOCKED**
pending confirmation, and Start must not be pressed before the user confirms
the target board, selected image and erase choice.

The MPU6050 and TFT are reportedly physically connected; the TFT power state
and both modules' electrical review remain uncertain. Isolate both modules
from the board-only test while that status is unresolved. Do not power the
trainer with the uncertain module connections in place. Programming and
physical LED behavior remain **not tested**.
