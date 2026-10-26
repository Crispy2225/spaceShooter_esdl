# Hardware assumption register — 26 October 2026

Scope: the three isolated tests in `firmware/tests/`. This records what their
source configures, what their interfaces merely propose, and what must be
checked on the fitted hardware. **No module connection or powered test is
electrically approved by this register.** `VERIFIED` means the stated
document/code fact was checked, never that a physical wire or module passed.

## Sources and status meanings

- **Lab manual:** `C:\Users\Laptop\OneDrive\Documents\MIT\5th Sem\ES\ICT 3144  ESD LAB MANUAL_2026.pdf`, PDF viewer pages 25–27 (Lab 6), 28–29
  (Lab 7), 37–41 (Lab 9, parallel LCD), and 63–65 (Appendix C). Its printed
  page number is four less than the PDF viewer page number in these sections.
- **Trainer manual:** `C:\Users\Laptop\Downloads\ARM_LPC-1768_User_Manual.pdf`,
  *ALS-SDA-ARMCTXM3-01-UM1*, PDF viewer pp. 20–21 (power/assembly), 24 (CNA),
  25 (CNC), 26 (connector sharing warning), 35 (CNC1 peripherals/jumpers),
  39 (placement). Its printed page number is one less. Connector tables are
  documentary evidence for candidate **board-side** signals only.
- **Existing project review:** `manual/hardware-readiness.md` and
  `manual/index.html`, especially physical-safety, P1, S1 and D1 sections.
  Photos identify labels but do not establish an electrically documented
  module variant, contact-1 orientation or current jumper positions.
- **MPU chip reference:** TDK InvenSense, *MPU-6000/MPU-6050 Register Map and
  Descriptions*, RM-MPU-6000A-00 rev. 4.2, §§4.17–4.19, 4.28 and 4.32:
  https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf.
  It documents the chip protocol, not the photographed breakout circuit.
- **Installed Keil files:** `C:\Keil\ARM\INC\NXP\LPC17xx\LPC17xx.h`,
  `C:\Keil\ARM\Startup\NXP\LPC17xx\startup_LPC17xx.s` and
  `system_LPC17xx.c`. The installed startup calls `SystemInit` before `main`;
  the installed system file defines `XTAL=12000000`, `CLOCK_SETUP=1` and PLL
  settings. Their compatibility with the fitted trainer is still unknown.

Status: `VERIFIED` = cited document or source fact only; `ASSUMPTION` = test
behavior chosen from a documented example but not observed; `UNVERIFIED` =
physical state or configuration lacks evidence; `BLOCKED` = must not use for
physical module work before review; `NOT APPLICABLE` = not used by this test.

## Test A — board-only smoke

`board_smoke/main.c` configures only GPIO0 P0.4–P0.11. The Lab 7 sample sets
`FIOSET` to turn each LED on and `FIOCLR` to turn it off. That is a **documented
intended polarity**, not a physical result. The program begins with all eight
outputs cleared, then sets the eight bits one at a time and clears them one
at a time. The delay is an uncalibrated volatile software loop.

| Signal / module pin | MCU port and function configured in code | Trainer contact documented | Source | Status | Check before board power/use |
| --- | --- | --- | --- | --- | --- |
| LED 1 input | P0.4 / GPIO output | CNA 1; proposed ribbon to CNA1 LED block | Trainer manual p24; lab manual Lab 7 pp28–29 | ASSUMPTION | Confirm chip/base identity, CNA/CNA1 orientation and actual LED routing. |
| LED 2 input | P0.5 / GPIO output | CNA 2; proposed CNA1 ribbon | Trainer manual p24; Lab 7 pp28–29 | ASSUMPTION | Same physical cable and route check. |
| LED 3 input | P0.6 / GPIO output | CNA 3; proposed CNA1 ribbon | Trainer manual p24; Lab 7 pp28–29 | ASSUMPTION | Same physical cable and route check. |
| LED 4 input | P0.7 / GPIO output | CNA 4; proposed CNA1 ribbon | Trainer manual p24; Lab 7 pp28–29 | ASSUMPTION | Same physical cable and route check. |
| LED 5 input | P0.8 / GPIO output | CNA 5; proposed CNA1 ribbon | Trainer manual p24; Lab 7 pp28–29 | ASSUMPTION | Same physical cable and route check. |
| LED 6 input | P0.9 / GPIO output | CNA 6; proposed CNA1 ribbon | Trainer manual p24; Lab 7 pp28–29 | ASSUMPTION | Same physical cable and route check. |
| LED 7 input | P0.10 / GPIO output | CNA 7; proposed CNA1 ribbon | Trainer manual p24; Lab 7 pp28–29 | ASSUMPTION | Same physical cable and route check. |
| LED 8 input | P0.11 / GPIO output | CNA 8; proposed CNA1 ribbon | Trainer manual p24; Lab 7 pp28–29 | ASSUMPTION | Same physical cable and route check. |
| CNA ground | No MCU port configured | CNA 10 / ground | Trainer manual p24 | UNVERIFIED | Confirm fitted cable/LED block and ground continuity unpowered. |
| Trainer supply | No module VCC selected by code | Board input, not a module rail | Trainer manual p20; lab manual Lab 6 p26 | UNVERIFIED | Review actual supply input, JP6/rails and board variant before power. |

`PINSEL0 &= 0xFF0000FF`, `FIODIR |= 0x00000FF0`, then `FIOSET/FIOCLR` are
adapted directly from Lab 7 pp28–29. Lab 6 p27 calls for CNA–CNA1 and reset
SW1 after programming. Neither that connection nor the expected active-high
LED result was observed in this preparation. A build or HEX does not verify
the cable, LED polarity or execution.

## Test B — MPU6050 standalone I²C

`mpu6050.c` implements chip register transactions, **not** an LPC1768 I²C
driver. `platform_unconfigured.c` returns failure from initialization and
performs no I²C read/write. Thus the code currently configures **no MCU I²C
pins, timing or UART output**. The following CNC entries are candidate board
signals from the trainer manual, not assignments implemented or approved.

| Signal / labelled module pin | MCU port and function configured in code | Candidate trainer contact | Source | Status | Check before module connection/power |
| --- | --- | --- | --- | --- | --- |
| MPU VCC | None; rail unknown | None allocated | Photo in hardware-readiness report; trainer manual p20 | BLOCKED | Exact breakout regulator/circuit, input range/current and reviewed supply rail. |
| MPU GND | None | CNC 10 / ground | Trainer manual p25; module photo | BLOCKED | Confirm contact 1, adapter continuity, common return and current path. |
| MPU SCL | None; P0.20/SCL1 is only a board candidate | CNC 6 | Trainer manual p25; manual/index.html S1 | BLOCKED | Breakout logic tolerance, pull-up value/rail, cable, routing and U16 RTC sharing. |
| MPU SDA | None; P0.19/SDA1 is only a board candidate | CNC 5 | Trainer manual p25; manual/index.html S1 | BLOCKED | Same review as SCL, including pull-ups and bus occupancy. |
| MPU AD0 | None | None allocated | TDK register map §4.32; breakout photo | BLOCKED | Establish actual bias/circuit and select the matching seven-bit address. |
| MPU XDA / XCL | None | None allocated | Breakout photo; hardware-readiness report | BLOCKED | Determine auxiliary-bus wiring and safe unused treatment. |
| MPU INT | None | None allocated | Breakout photo; hardware-readiness report | BLOCKED | Determine whether needed, output characteristics and any GPIO route. |
| Diagnostic output | None; no UART pin or interface selected | None established | manual/index.html P1/S1 | BLOCKED | Prove a board output path, connector and signal levels; RS-232 CN9 is not automatically MCU-level UART. |
| Sample timebase | None; stub returns zero and never passes init | Not applicable | `platform.h`, `platform_unconfigured.c` | BLOCKED | Implement and verify monotonic timestamps and sample pause. |

Chip-protocol choices from RM-MPU-6000A-00: seven-bit address candidates
0x68/0x69 depend on AD0 (§4.32); WHO_AM_I is register 0x75 with documented
default value 0x68 (§4.32); PWR_MGMT_1 is 0x6B (§4.28). The test **chooses**
to write 0x00 there, clearing SLEEP and selecting the internal oscillator;
this is an `ASSUMPTION` about the desired diagnostic operating mode, not a
measured clock or physical calibration. ACCEL_XOUT_H starts at 0x3B; a
14-byte consecutive read crosses two temperature bytes and ends at gyro Z
low byte 0x48 (§§4.17–4.19). These register facts are `VERIFIED` against
the chip reference; the fitted IC identity remains `UNVERIFIED` until a
matching WHO_AM_I and physical marking are observed.

The eventual bus port must select the reviewed pins/peripheral, calculate
timing from a checked clock, perform register-pointer write plus complete
read, and use finite timeouts. It must return distinct NACK/timeout errors,
STOP/recover the bus on failures, and never return success for partial data.
No bus speed, pull-up configuration, sampling interval, UART pins or baud
rate is chosen tonight. The two's-complement raw conversion in `mpu6050.c`
does not establish axis orientation or calibrated units.

## Test C — TFT standalone SPI structure

The photographed PCB says `1.8 TFT SPI 128*160 V1.1`; its controller is
unidentified. The test requests a black fill, white text and four RGB565
corner marks at coordinates within 128x160. Those colors, dimensions and
origin are an `ASSUMPTION` for a future driver review; no command byte,
controller initialization, rotation, offset or physical screen result is
claimed. `tft_port_unconfigured.c` fails initialization and sends **no SPI**.

| Signal / labelled module pin | MCU port and function configured in code | Candidate trainer contact | Source | Status | Check before module connection/power |
| --- | --- | --- | --- | --- | --- |
| TFT VCC | None; rail unknown | None allocated | Hardware-readiness report and module photo | BLOCKED | Exact module/controller circuit, input range/current and approved rail. |
| TFT GND | None | CNC 10 / ground | Trainer manual p25; module photo | BLOCKED | Confirm adapter, return path and sharing with MPU ground plan. |
| TFT SCK | None; P0.15/SCK0 is only a board candidate | CNC 1 | Trainer manual p25; manual/index.html D1 | BLOCKED | Controller mode/frequency/logic, cable, U5/U6 sharing and JP12 state. |
| TFT SDA (SPI data input) | None; P0.18/MOSI0 is only a board candidate | CNC 4 | Trainer manual p25; module photo | BLOCKED | Verify input direction, controller logic and any U5 sharing. This is not I²C SDA. |
| TFT CS | None; P0.16/SSEL0 is only a board candidate | CNC 2 | Trainer manual p25; manual/index.html D1 | BLOCKED | Select allocation, polarity/timing, U5/U6 sharing and JP13 state. |
| TFT RESET | None | None allocated | Module photo; hardware-readiness report | BLOCKED | Controller-specific reset polarity/timing and reviewed GPIO or circuit treatment. |
| TFT A0 (data/command) | None | None allocated | Module photo; hardware-readiness report | BLOCKED | Controller-specific polarity and reviewed GPIO allocation. |
| TFT LED (backlight) | None | None allocated | Module photo; hardware-readiness report | BLOCKED | Backlight supply/current limit or driver circuit; lit LED alone does not prove SPI. |

SPI mode, frequency, chip-select polarity, reset sequence, data/command
polarity, display controller, backlight drive and panel offsets are all
`BLOCKED`. Lab manual Lab 9 pp37–41 documents a different parallel 16x2 LCD
and cannot resolve them. No ST7735 assumption is made.

## Shared clock, programming and connector gates

| Choice or dependency | Source and exact reference | Status | Required check |
| --- | --- | --- | --- |
| MCU startup calls `SystemInit` before `main`; test mains call `SystemCoreClockUpdate` once | Installed `startup_LPC17xx.s` Reset_Handler; installed `system_LPC17xx.c`; lab manual Lab 7 p28 places both calls in `main` | UNVERIFIED | The source behavior is checked; confirm the actual chip and startup pairing before flash. |
| Installed system file assumes 12 MHz `XTAL` with PLL setup; lab ISP procedure also says 12 MHz oscillator | Installed `system_LPC17xx.c` definitions around lines 295–305 and 395; lab manual Lab 6 p26 | ASSUMPTION | Match fitted oscillator/crystal and clock tree to revision-matched board documentation; review Keil target memory/clock settings. |
| Board smoke delay count 1,000,000 | `board_smoke/main.c`; Lab 7 pp28–29 uses an uncalibrated loop of 10,000 | ASSUMPTION | Observe speed on the actual board; it is not a timer or calibrated millisecond value. |
| Programmer, ISP pins and baud | Lab manual Lab 6 pp26–27: CN9/SW21/JP3/Flash Magic 6.01, 9600 in main settings and 115200 in advanced settings | UNVERIFIED | Identify actual lab tool/cable and resolve conflicting baud directions with operator; do not copy switch/jumper settings blindly. |
| CNC/CNC1 shared bus | Trainer manual pp26,35; hardware-readiness report | BLOCKED | Photograph ribbons and JP12/JP13; check U5 EEPROM, U16 RTC and U6 RS485 before either module or simultaneous use. |
| Physical connector orientation | Trainer manual pp24–26 and placement p39; hardware-readiness report | BLOCKED | Identify CNC/CNA contact 1, cable key/stripe, adapter contact map and unpowered continuity. |
| Module voltage and logic compatibility | Trainer manual p20 gives board supply architecture only; hardware-readiness report module inventory | BLOCKED | Obtain exact breakout schematics or traced circuits and signed electrical review. No connector table authorizes module power. |
| Simultaneous MPU/TFT/UART operation | manual/index.html I1; hardware-readiness report | BLOCKED | Both standalone physical passes, then reviewed combined pin, current, timing and bus plan. |

The test suite contains no executable MCU I²C/SPI timing configuration, no
verified diagnostic UART, no electrical approval, and no physical pass.
