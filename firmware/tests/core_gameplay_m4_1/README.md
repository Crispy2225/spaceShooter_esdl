# Flick & Fly M4.1 — calibration reliability and visual polish

## Diagnosis and baseline

This isolated image derives from `../core_gameplay_m4/main.c`. The current M4 startup stops forever if the MPU wake/setup write or **any one** of its 32 calibration reads fails. It also stops if the raw span exceeds 600 counts on any axis. Thus a transient I²C read fault or a moved sensor during collection plausibly explains the reported intermittent immobile ship. The stored source and build log cannot distinguish those causes on the physical board. M4 did not show separate retry/timeout states or recover from a failed startup attempt. Its normal gameplay read failure already ignored partial bytes and froze ship movement; that behavior is retained.

The exact TFT and I²C2 driver functions, pin assignments, ST7735S configuration, MPU6050 register writes, `MOVE_AXIS=0`, `MOVE_SIGN=+1`, `DEAD_ZONE=150`, and `COUNTS_PER_STEP=1500` match M4. The M4 source's note that the axis/sign were provisional remains relevant unless the owner's physical M3/M4 testing established a measured mapping. M4 was reported smooth on hardware; M4.1 has not been flashed or tested.

## Startup calibration

The explicit states are `CAL_SETUP → CAL_SETTLE → CAL_COLLECT → CAL_READY`, with `CAL_RETRY` and terminal `CAL_FAILED` paths. Each setup attempt initializes I²C2, wakes the sensor, retains the reference wake delay count, and writes the same three MPU settings. The `KEEP STILL` message stays visible during the existing 2-second settling period. Collection requests one sample no sooner than every 20 ms. `KEEP STILL 08/32` through `KEEP STILL 32/32` shows progress every eight **valid** reads. Failed reads increment a separate counter; partial bytes are never decoded or added to the sums.

Each attempt requires exactly 32 valid X/Y/Z samples, allows at most 7 failed reads, and has a 2,500 ms collection deadline. It checks each axis's max-minus-min span against the same 600 raw-count bound used by M4. This retains M4's stability policy: the reported stationary variation was about ±50 counts, while deliberate tilt was reported in thousands. A bounded set of 32 samples covers roughly 620 ms at 20 ms spacing, enough to average stationary noise without prolonging startup. The offsets are signed arithmetic means stored only in RAM. `READY` appears only after all reads and span checks pass; gameplay starts then.

Wake/setup failures, eight failed collection reads, collection timeout, or unstable samples show `MPU WAKE ERROR`, `MPU SETUP ERROR`, `CAL READ ERROR`, `CAL TIMEOUT`, or `CAL UNSTABLE` in red. After 500 ms, the firmware reinitializes the sensor and tries again, at most **three total attempts**. A third failure leaves its cause visible and replaces the lives line with `RESET TO RETRY`; the game does not start. Power cycle/reset while holding the intended neutral orientation to retry. No automatic recalibration occurs during gameplay and no button input was added. The existing live read error freezes ship movement, displays `MPU READ ERROR`, and resumes on a valid reading.

## Visual changes and workload

The 128×160 portrait layout and all entity dimensions remain unchanged. The ship gains a small blue cockpit inside its cyan/white/red silhouette. The green 11×9 enemy uses a fixed row mask with red eyes. Fourteen dim one-pixel stars are drawn once at startup by a deterministic coordinate rule; `grid_pixel` uses the same rule to restore them beneath moving sprites. The HUD gets a thin blue divider. Routine score/life updates redraw only the five score digits and seven-character lives field; the full 128×32 HUD band is filled only at startup. `GAME OVER` gains a simple frame and `RESET TO PLAY`. A player collision briefly shows `HIT!` for 250 ms in the bottom status band, unless a sensor error has priority.

M4's fixed pools, collision rules, enemy/shot timing, and partial-window redraw order are retained. A maximum gameplay frame that moves the ship and redraws all three enemies and one projectile restores and paints at most **1,134 entity pixels** (510 ship + 594 enemies + 30 projectile), or 2,268 RGB565 data bytes, plus window commands. A simultaneous HUD update adds at most 576 text-window pixels rather than M4's 4,096-pixel HUD clear; a rare hit/status change adds its status window. Stars add 14 tiny startup writes and only integer arithmetic in background restoration, with no per-frame full-screen redraw. The legacy 0.75 MHz SPI transfer and bounded polling still make the nominal 40 ms frame an estimate; a physical timing/flicker check is required.

## Build and checks

`build.py` is a new-folder copy of M4's recovered ARMCC 5.03 Build 24 script. It verifies the vector patch against the preserved combined-test raw/final pair, refuses an existing output directory, and logs exact compile/assemble/link/FromELF commands. From the repository root, choose a new directory for any rebuild:

```powershell
C:\Users\Laptop\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe firmware\tests\core_gameplay_m4_1\build.py --output-dir firmware\build\2026-10-29\core_gameplay_m4_1_rebuild_1
```

The reviewed candidate is `../../build/2026-10-29/core_gameplay_m4_1_final/core_gameplay_m4_1_lpc1768.hex`, **20,220 bytes**, SHA-256 `571C74AC8DEAEAEAA8B77C575C8436B0A5C954FA21419D6BA946646A5C33E73B`. The final build emitted no warnings or errors. The first `core_gameplay_m4_1/` output is preserved; the final output includes the subsequent HUD optimization. Only the final `*_lpc1768.hex` has the LPC1768 vector patch and is the programming candidate.

## Physical test for the owner

1. Confirm the existing working M4 wiring, supply, backlight, Flash Magic settings, and exact final M4.1 HEX filename/hash before programming. Keep the M4 HEX available for comparison; save the verify log.
2. Reset while holding the sensor in its intended neutral playing position. Watch `KEEP STILL`, progress through 32 valid samples, then `READY`. Repeat at least ten cold starts and ten resets; record success count, displayed errors, elapsed startup time, and whether the ship moves correctly after each success.
3. During separate starts, deliberately move the sensor through calibration and observe `CAL UNSTABLE` followed by a retry. If safe on the existing setup, briefly interrupt sensor communication to exercise wake/setup/read errors and retry. After three failed attempts, confirm the cause and `RESET TO RETRY` remain visible and the ship does not move. Reset after restoring the sensor.
4. Compare neutral jitter, left/right direction, sensitivity, projectile cadence, enemy motion, hits, score/lives, and game over against known-good M4. Watch the cockpit, enemy eyes, star cleanup, HUD legibility, hit feedback, trails, frame pauses, and flicker. Note whether any polish harms responsiveness.
5. Report the exact flashed hash, observations, photos/video, and any failure code. Build success alone does not establish a physical fix.

No M5 features are included.
