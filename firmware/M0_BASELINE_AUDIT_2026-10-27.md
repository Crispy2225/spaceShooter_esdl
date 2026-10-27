# M0 repository audit — 27 October 2026

## Evidence boundary

This is a read-only firmware and documentation audit. No compile, flash, electrical measurement, or physical test was performed. The owner reports that MPU6050 readings appeared on the physical ST7735S TFT while both operated together. The exact flashed image, wire endpoints, voltages, and session log remain unrecorded. File hashes identify bytes, not physical correctness.

## Actual baseline and build

| Item | Inspected fact |
| --- | --- |
| Combined source | [`tests/mpu6050_tft_test/main.c`](tests/mpu6050_tft_test/main.c), 13,345 bytes; SHA-256 `1BEF2332F77804412D5E9BEEE2EDBC07C5CCEB691DD10CA2F226FA4C473AC997` |
| Candidate HEX | [`build/2026-10-27/mpu6050_tft_test/mpu6050_tft_test_lpc1768.hex`](build/2026-10-27/mpu6050_tft_test/mpu6050_tft_test_lpc1768.hex), 13,776 bytes; modified 27 October 2026 12:58:47 local; SHA-256 `6E74546A816579CB2D354DB6DB2015BAB7C0D74CE7B639B10A1CD65F3C950700` |
| Other saved output | Same directory has `mpu6050_tft_test.axf` (20,280 bytes) and `mpu6050_tft_test_raw.hex` (14,091 bytes); retain both. The final HEX has the LPC1768 vector checksum patch recorded as `0xEFFFF44E`. |
| Build evidence | [`build.log`](build/2026-10-27/mpu6050_tft_test/build.log) names this source and final HEX, their matching current hashes, ARMCC 5.03/Cortex-M3, successful compile/assembly/link/HEX conversion, 315 valid Intel HEX records and 4,992 data bytes. It records no exact command lines or complete target options. Source and HEX timestamps are consistent with the logged sequence, but this is not independent proof of a bit-for-bit rebuild or of the image flashed to the trainer. |
| Toolchain | Earlier [`VALIDATION_2026-10-27.md`](VALIDATION_2026-10-27.md) records Keil µVision 4.70.0.0 and ARMCC 5.03 Build 24. The combined log says 5.03, without a build number. |

The required project inputs identified by the combined log and source are this `main.c`, installed `C:\Keil\ARM\Startup\NXP\LPC17xx\startup_LPC17xx.s` (SHA-256 `82618242DBB65CC72375FFC3B5F8F27798BD110FEC30DA7F671F9F0250EDBF3D`), `system_LPC17xx.c` in that directory (SHA-256 `D474807C1A8F9F5EFFDCEA23284CAC7B40384F4D4042C5F753A7C026504A0383`), installed `C:\Keil\ARM\INC\NXP\LPC17xx\LPC17xx.h` (SHA-256 `7ABD2DC75636A25C916238587C4EFF65945328501B8B1DF934053B592964568C`), the associated `system_LPC17xx.h`, CMSIS headers, and ARMCC C library headers used by `stdio.h`. The source contains the TFT and I²C2 code in one file; the earlier standalone MPU and TFT stub sources are not combined-build inputs. The installed startup calls `SystemInit` before `main`. The installed system source specifies a 12 MHz crystal, PLL setup and nominal 96 MHz core clock; the fitted oscillator and actual clock were not measured.

**Build reproducibility:** There is no combined-test `.uvproj`/`.uvprojx`, build script, response file, or full command transcript in its test/build directories. `tests/test3/test.uvproj` has an empty source group. `tests/tft_test2/test2.uvproj` targets a different standalone test and must not be treated as the combined target. Earlier standalone logs show Cortex-M3, `-O0`, C99, Keil include roots, flash base `0x00000000`, RAM base `0x10000000`, and `Reset_Handler`, but those exact options cannot be attributed to the combined build solely from its log. Therefore the precise combined Keil target and complete compiler/link configuration are **unresolved**. The existing HEX is a preserved candidate, not a freshly rebuilt or physically verified artifact.

## Source-verified peripheral configuration

| Subsystem | Source finding | Physical status |
| --- | --- | --- |
| I²C2 | PCONP bit 26; P0.10 SDA2 and P0.11 SCL2 via PINSEL0 function 2, open drain, PINMODE0 function 2; PCLKSEL1 bits 21:20 cleared (core/4); I2SCLH/I2SCLL each 125. At nominal 96 MHz core this implies approximately 96 kHz SCL, without measurement. | Owner reports working tilt readings. Actual CNA 7/8 versus CN6 7/8 route, pull-ups, AD0 wiring, supply and rail are unresolved. |
| MPU read | Seven-bit address `0x68` (`0xD0` write, `0xD1` read). Writes `0x00` to `0x6B`, then `0x04` to `0x1A`, `0x09` to `0x19`, `0x00` to `0x1C`. Repeated-start six-byte read from `0x3B` forms signed raw X/Y/Z. Polls with finite software timeouts; no gyro read or calibration. | Owner reports live values on TFT; no transaction capture or flashed-image identification. |
| TFT SPI | Legacy SPI, not SSP: PCONP bit 8; P0.15 SCK and P0.18 MOSI via function 3. PCLKSEL0 bits 17:16 cleared (core/4), SPCR `0x20` (master, mode 0, MSB first, eight bits), SPCCR 32; nominal SPI clock approximately 0.75 MHz. | Owner reports working display; no measured clock or current cable map. |
| TFT GPIO | P0.16 CS, P0.21 RESET and P0.22 A0/DC are GPIO outputs. Source comment proposes CNC 2/7/8 respectively; CNC 1 SCK, CNC 4 DIN and CNC 10 GND are source/manual candidates. | Individual TFT contacts, VCC, GND and LED/backlight route remain unverified physically. |
| Display init/draw | High–low–high reset with 100 ms waits; ST7735S register sequence; `COLMOD=0x05` RGB565, portrait `MADCTL=0xC8`, X/Y RAM offsets 1/2; 128×160 black clear, title and X/Y/Z text, about 200 ms update delay. | Successful physical function is user-reported; precise orientation/offset observation is not archived. |

The trainer manual *ALS-SDA-ARMCTXM3-01-UM1* PDF pp. 24–26 supports CNA 7/8 and CN6 7/8 for P0.10/P0.11, and CNC 1/2/4/7/8/10 for the TFT candidate map. PDF p. 35 describes optional CNC–CNC1 sharing with onboard SPI/RS485/I²C and JP12/JP13 selection. These are **documented connector functions**, not proof of installed wires or jumper positions. [`manual/hardware-connections.md`](../manual/hardware-connections.md) has per-module signal rows, evidence and unresolved power/ground/backlight details; its candidate HEX wording was clarified in this audit. Do not derive a module VCC voltage from MCU logic voltage.

## Discrepancies and next action

[`manual/index.html`](../manual/index.html), [`manual/hardware-readiness.md`](../manual/hardware-readiness.md), and [`HARDWARE_ASSUMPTIONS.md`](HARDWARE_ASSUMPTIONS.md) describe the earlier 26 October pre-test state: no local firmware/physical pass, an unconfigured I²C1/CNC 5–6 proposal, and unidentified TFT controls. They are historical and conflict with the later source and owner report if read as current status. The separate 16×2 LCD test and its P0.23–P0.28 wiring are not this SPI TFT test. The owner report establishes simultaneous physical function, but does not establish that this repository candidate HEX was flashed or that every proposed connector contact was used.

For preservation, keep the combined source, final HEX, raw HEX, AXF and log unchanged. Place any future dated archive **outside firmware build inputs** with a manifest of these SHA-256 hashes and read-only copies; avoid overwriting either the repository artifacts or `C:\Users\Laptop\Downloads\main.c` (reference SHA-256 `AB3AC2F6020E7735432CEB1880478CE21C9BC3BCFA99DA952E7012E593138B87`). For M1, first record the actual flashed HEX or Flash Magic log, a screen photo showing live values, and the installed wire/rail/jumper map. Then create a separate reproducible combined-test target before any game or calibration work.
