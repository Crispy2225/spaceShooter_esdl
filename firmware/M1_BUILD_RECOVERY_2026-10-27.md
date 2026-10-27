# M1 ARMCC build recovery — 27 October 2026

## Result and limits

The combined test has no saved Keil target, but it **can be reproduced to the exact final HEX bytes** using an ARMCC command sequence supported by the repository's earlier standalone build logs. The recovered compile/link pattern produced a raw combined HEX whose SHA-256 exactly matches the preserved raw HEX (`FA88E4B43C9647D164F133DA2715AD3A878A12B6AB5816E18A67AA1B19190418`). The vector-checksum procedure documented in [`VALIDATION_2026-10-27.md`](VALIDATION_2026-10-27.md) transforms that raw HEX to bytes identical to the preserved final combined HEX (SHA-256 `6E74546A816579CB2D354DB6DB2015BAB7C0D74CE7B639B10A1CD65F3C950700`). The isolated recovery inputs and output are in [`build/2026-10-27/recovery_combined/`](build/2026-10-27/recovery_combined/); no original artifact was overwritten.

This establishes a **reproducible command sequence for the saved HEX bytes**, not the original operator's exact command history, a saved µVision target, the identity of the image flashed to the board, or physical operation. The recovered `.o` and `.axf` files are not byte-identical to the saved originals; the raw HEX is. The owner-reported combined physical result remains separate evidence.

## Configuration recovered from repository evidence

| Setting | Evidence-backed value |
| --- | --- |
| Toolchain | `C:\Keil\ARM\ARMCC\bin\armcc.exe`, `armasm.exe`, `armlink.exe`, `fromelf.exe`, all 5.03 Build 24 MDK-ARM Lite (`--vsn` recorded in M1 `build.log`); µVision 4.70 is installed, but no combined `.uvproj` exists. |
| Target / inputs | LPC1768, Cortex-M3; one selected application `main.c`; installed `C:\Keil\ARM\Startup\NXP\LPC17xx\startup_LPC17xx.s` and `system_LPC17xx.c`; installed `LPC17xx.h`, `system_LPC17xx.h`, CMSIS headers and ARMCC C library/`stdio.h`. No M1 application needs the earlier standalone stub sources. |
| C compile | `--cpu=Cortex-M3 -O0 --c99 --apcs=interwork -IC:\Keil\ARM\INC\NXP\LPC17xx -IC:\Keil\ARM\CMSIS\Include -c`; no extra `-D` appears in the logged standalone commands. |
| Assembly | `armasm --cpu=Cortex-M3` on the installed startup source. Its `Reset_Handler` calls `SystemInit` before `main`. |
| Link | `armlink --cpu=Cortex-M3 --ro-base=0x00000000 --rw-base=0x10000000 --entry=Reset_Handler --no_remove --first=startup_LPC17xx.o(RESET)` with startup, system and selected main objects; standard ARMCC libraries, no scatter file or custom library option in the command. This matches multiple successful standalone logs and reproduced the combined raw HEX. |
| Clock | Installed `system_LPC17xx.c` selects 12 MHz XTAL and PLL nominal 96 MHz core. Actual fitted oscillator and runtime clock remain unmeasured. |
| HEX | `fromelf --i32combined --output <raw.hex> <target.axf>`, then put the little-endian two's complement of vector words 0–6 at offset `0x1C`, recompute the affected Intel HEX record checksum, emit valid ASCII HEX records. The recovered routine regenerates the existing final combined HEX exactly. This operation is necessary because saved raw files have zero in vector word 7. |

The prior [`board_smoke_single/build.log`](build/2026-10-27/board_smoke_single/build.log), [`lcd_test/build.log`](build/2026-10-27/lcd_test/build.log), and [`mpu6050_lcd_test/build.log`](build/2026-10-27/mpu6050_lcd_test/build.log) record the command pattern. `tests/test3/test.uvproj` has no source input. `tests/tft_test2/test2.uvproj` targets a different standalone test with an unconfigured TFT port; it is not a suitable combined or M1 target. No project scatter file, combined `.uvprojx`, or saved post-build checksum script was found. The checksum was recovered from the documented formula and raw/final byte comparison, then verified in the new build script against the preserved files before building M1.

## M1 dependency and build check

[`tests/mpu6050_tilt_calibration/main.c`](tests/mpu6050_tilt_calibration/main.c) contains the same copied I²C2 and TFT driver section as the combined source (text comparison exact), then its separate calibration logic. It needs only that `main.c`, the installed system/startup files and their headers/libraries. The successful link found no missing symbols or duplicate definitions. There is one `main` and one strong `SysTick_Handler`; startup's interrupt handler is weak. The new source was not changed during this recovery task.

The isolated [`tests/mpu6050_tilt_calibration/build.py`](tests/mpu6050_tilt_calibration/build.py) is the reproducible M1-only build configuration. It invokes the commands above with the M1 source, records all arguments and exit codes, and refuses to reuse an output directory. Run it with `C:\Users\Laptop\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe`; for any later run provide a **new** `--output-dir` under `firmware/build`. It first proves its checksum implementation reproduces the preserved combined final HEX. The exact M1 transcript is [`build/2026-10-27/mpu6050_tilt_calibration/build.log`](build/2026-10-27/mpu6050_tilt_calibration/build.log).

All M1 compile, assembly, link and FromELF steps exited 0, with **no emitted warnings or errors**. The final [M1 HEX](build/2026-10-27/mpu6050_tilt_calibration/mpu6050_tilt_calibration_lpc1768.hex) is **16,144 bytes**, SHA-256 `5C804111DF11D29FBB19BD29AF56B4E8329106DF2E5A92D4868D6B7B3B5E2F77`. It has **369 valid Intel HEX records**, one EOF, a vector checksum word `0xEFFFF406`, and vector words 0–7 sum to zero modulo 2³². Its data addresses observed in the HEX range from `0x00000000` through `0x000016DB`. These are file-structure checks, not proof of hardware compatibility or successful execution.

## Remaining evidence and action

The exact previously flashed combined HEX, physical module contacts and supply/backlight routes, present jumper state, actual oscillator and M1 physical behavior are unresolved. No board was flashed or powered in this task. Review the M1 source, output hash and actual assembled wiring before programming; then perform the [five-cycle calibration protocol](tests/mpu6050_tilt_calibration/README.md) and save the Flash Magic log and observed screen evidence. The known combined source and final HEX retain their M0 hashes; `C:\Users\Laptop\Downloads\main.c` and `audit.sqlite` were not changed.
