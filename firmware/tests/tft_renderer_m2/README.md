# TFT renderer diagnostic — M2

## Scope and evidence

`main.c` is a standalone LPC1768 display test. Its legacy SPI transport, GPIO control, ST7735S initialization, 128×160 RGB565 window/fill functions, 5×7 text, and SysTick delay are copied from `../mpu6050_tft_test/main.c`. The M2-only drawing functions are below that copied section. No MPU6050 transactions or gameplay are present.

Source settings: P0.15 SCK and P0.18 MOSI use legacy SPI function 3; P0.16 CS, P0.21 RESET, and P0.22 DC use GPIO. CS and RESET are active low; DC low selects commands. SPI is master mode 0, 8-bit MSB first, with PCLK=core/4 and divider 32 (nominal 0.75 MHz if the core is 96 MHz). Reset goes high–low–high with three 100 ms waits. The init writes `COLMOD=0x05` (RGB565), `MADCTL=0xC8`, then sleep-out and display-on. Logical portrait bounds are x=0..127, y=0..159; the existing window routine adds RAM offsets (1,2). These values are source verified, while the actual module controller variant, orientation, offsets, fitted oscillator, electrical wiring, supply, and backlight behavior are **not** physically verified. The connector mapping in the combined source is proposed; see `../../M0_BASELINE_AUDIT_2026-10-27.md` and `../../../manual/hardware-connections.md`. M2 introduces no pins, peripherals, or polarity changes.

## Visible sequence

On reset the screen clears to black. RGB565 red, green, blue, and white bars remain visible at the top. Color coded 2×2 corner markers and TL/TR/BL/BR labels expose orientation and edge clipping. A grid covers y=88..143. A 17×15 spaceship moves across that grid at y=108, with x kept between 8 and 103. The first sweep is labelled `PARTIAL`: each move restores only the previous 17×15 window from the same mathematical grid pattern used for the static screen, then draws the ship in its new window. The two windows can overlap safely because restoration happens first. It then pauses on a `FULL` reference screen rebuilt with a whole-screen clear and the ship at x=55. This repeats. Each movement is separated by 180 ms, plus display transfer time; each reference pauses for 2 s. The 17×15 sprite uses row masks and RGB565 fills; no framebuffer is allocated.

SPI transfer or window failures stop execution because the display cannot report its own failure reliably. There is no board-side input to change modes. The full-screen reference is intentionally infrequent and is not used for movement updates.

## Build

`build.py` is an M2-specific copy of the recovered M1 command procedure. It validates the checksum patch against the preserved combined-test raw/final pair, checks installed inputs, logs ARMCC 5.03 Build 24 versions and commands, and refuses any existing output directory. From the repository root:

```powershell
C:\Users\Laptop\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe firmware\tests\tft_renderer_m2\build.py --output-dir firmware\build\2026-10-29\tft_renderer_m2_rebuild_1
```

The default output is `firmware/build/2026-10-29/tft_renderer_m2/`. That initial build is preserved; it logged one unused copied helper warning. The unused helper was removed from M2 only, then a clean build was run with `--output-dir firmware/build/2026-10-29/tft_renderer_m2_final`. Its [final HEX](../../build/2026-10-29/tft_renderer_m2_final/tft_renderer_m2_lpc1768.hex) is 11,120 bytes, SHA-256 `9B3EF1D9A4CAEEC07354BE7A741260BB91C2B8F56680B3E16AC52DBCC13A0FFE`; [build.log](../../build/2026-10-29/tft_renderer_m2_final/build.log) has no warnings or errors. The HEX contains 255 valid Intel HEX records, one EOF, 4,024 data bytes, addresses 0x00000000..0x00000FB7, and vector word 7 = `0xEFFFF486`; the first eight vector words sum to zero modulo 2^32.

For a later rebuild use a new path below `firmware/build`, e.g. `--output-dir firmware/build/2026-10-29/tft_renderer_m2_rebuild_1`. Only the final `*_lpc1768.hex` has the LPC1768 vector checksum patch. The raw HEX is retained for audit and is not the programming candidate.

## Physical check for the owner

Before programming, verify the actual board, display pinout, supply, backlight, connector routing, and established Flash Magic settings against the installed wiring. Select the reviewed final M2 HEX and record its SHA-256. Photograph the startup screen and both `PARTIAL` and `FULL` phases. Confirm red/green/blue/white bars, distinct corner markers, readable corner labels, and the full grid, including the left/right edges. Watch several sweeps for correct horizontal motion, clipping, trails, damaged grid lines, flicker, and differences from the full redraw. Record any offset or color/order mismatch and the exact flashed file. Build success alone does not verify the physical display.
