# Flick & Fly M3 — tilt movement and shooting

## Scope and evidence

This is an isolated LPC1768 test. The TFT transport, ST7735S initialization, RGB565 window operations, text, M2 ship sprite, and background-restoring partial redraw derive from `../tft_renderer_m2/main.c`. The I²C2 transactions and MPU6050 register configuration derive from `../mpu6050_tft_test/main.c`. M2 was physically tested by the owner. M3 has only been built and structurally checked; it has not been flashed or observed on the board.

The recorded observations are roughly ±50 stationary raw counts and deliberate tilt changes of thousands of counts. No saved left/neutral/right values establish the correct movement axis or sign. `MOVE_AXIS=0` (X) and `MOVE_SIGN=+1` are **provisional**, not calibration-derived. Change these two definitions near the top of `main.c` if physical testing shows another axis or reversed motion. The sensor remains on I²C2 at P0.10/P0.11 and address 0x68; the TFT remains on the M2 legacy SPI/GPIO pins. The documented TFT connector contacts include unverified proposed RESET/DC routes; review the actual wiring before flashing.

## Behavior

On startup the display says `HOLD STILL` for 2 seconds. The firmware then requires 32 successful X/Y/Z reads spaced by 20 ms. It averages each axis into RAM-only neutral offsets. A read error or a greater than 600-count span on **any** axis shows a red calibration error and stops; reset while holding the desired neutral orientation to retry. A passed span check cannot prove that the sensor was perfectly still.

The ship starts at x=55 and y=108. Every nominal 40 ms frame, a fresh selected-axis reading is compared with its neutral offset. `MOVE_SIGN` determines direction. Deltas inside ±150 raw counts leave the ship still. Beyond the dead zone, movement is 1 pixel plus 1 pixel per additional 1500 counts, capped at 5 pixels per frame. `DEAD_ZONE` and `COUNTS_PER_STEP` are provisional sensitivity settings; adjust from physical observations. The 17×15 sprite's x is clamped to 8..103, so it stays fully inside the 128×160 portrait screen. The old ship window is restored from the mathematical grid before the new ship is drawn. The scene is drawn once; no full-screen framebuffer or repeated full-screen redraw is used.

One yellow 3×5 projectile is fired from the ship center when at least 600 ms have elapsed since the last shot **and** no projectile is active. It moves upward 4 pixels per frame. Each old projectile window is restored from the grid before the new one is drawn; it disappears at the top of the play area and its single slot is reused. The projectile stays above the ship and has no collision or scoring behavior. There is no button or extra GPIO.

On a live sensor read failure, the ship freezes, the partial data is ignored, and `MPU READ ERROR` appears in red. A later valid read restores `I2C OK` and movement. Projectile animation can continue during a sensor error. SPI/I²C operations use the existing bounded polling routines. Display transfer and an I²C timeout can lengthen a frame; timing is therefore nominal, not a guaranteed 25 fps. The 2-second hold and sample spacing are startup-only blocking waits.

## Build and artifact

`build.py` is an isolated copy of M2's recovered ARMCC 5.03 Build 24 procedure. It verifies the LPC1768 checksum patch against the preserved combined-test raw/final HEX pair, compiles and links the M3 source with installed startup and system files, runs FromELF, patches vector word 7, and refuses an existing output directory. From the repository root, choose a **new** path under `firmware/build`:

```powershell
C:\Users\Laptop\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe firmware\tests\tilt_shooter_m3\build.py --output-dir firmware\build\2026-10-29\tilt_shooter_m3_rebuild_1
```

The reviewed final output is `../../build/2026-10-29/tilt_shooter_m3_final/tilt_shooter_m3_lpc1768.hex`, **14,216 bytes**, SHA-256 `8938FC7935FEAC3A03731E1D1801581311983D08574C0127A4215995FB5614EE`. `build.log` in that directory records tool versions, exact commands, and the vector checksum. The earlier `tilt_shooter_m3/` output is retained because the source was subsequently changed to restore the reference's exact wake delay. Program only the final `*_lpc1768.hex`; the raw HEX lacks the LPC1768 vector checksum patch.

## Physical test for owner

1. Check actual TFT and MPU wiring, power, backlight, jumper state, and Flash Magic settings against `../../../manual/hardware-connections.md`. Record the exact final HEX filename and hash used.
2. Hold the sensor in the intended neutral playing orientation from reset through `CAL OK`. If `CAL READ ERROR` or `CAL MOVED RESET` appears, reset and retry; do not interpret it as a valid offset.
3. At neutral, watch for drift or jitter. Tilt left, return neutral, tilt right, and return neutral repeatedly. Record which direction the ship moves and the approximate tilt needed. If reversed, change `MOVE_SIGN`; if the selected axis does not separate directions, change `MOVE_AXIS`, rebuild to a new output directory, and repeat. Adjust dead zone and sensitivity from measured behavior.
4. Watch the yellow projectile fire automatically, move upward, disappear, and fire again. Inspect the ship and projectile paths for grid trails, edge clipping, flicker, and overlap. Verify the ship remains within the screen.
5. If a sensor connection can be safely interrupted in the existing setup, check that `MPU READ ERROR` appears and the ship freezes; after reconnection, confirm recovery. Record any hangs, display errors, or timing issues. Report the exact flashed hash and photos/video before claiming physical success.

M4 features are outside this test.
