# Flick & Fly M7 — isolated decision-tree steering candidate

**Status: software built and host tested; owner-reported board operation.** This folder is an isolated copy of the M5 game with the M6 three-class tilt tree added. M4.1 and M5 source, builds, and historical records remain untouched. The owner now reports flashing the M7 candidate and observing gameplay that appears the same as M5. No Flash Magic log, board readback, dated trial record, photo or measured response was supplied, so the observation is not an instrumented feature-by-feature validation.

## Source change and behavior

M5 reads six accelerometer bytes over I²C2, decodes signed X/Y/Z values, averages 32 valid neutral samples, and computes `delta = ((long)raw[MOVE_AXIS] - neutral[MOVE_AXIS]) * MOVE_SIGN` with `MOVE_AXIS=0`, `MOVE_SIGN=+1`. On a successful gameplay read, M5 holds position for `-150 <= delta <= 150`; outside that zone it moves left/right by `1 + (abs(delta)-150)/1500` pixels, capped at 5 and clamped to ship bounds 8–103. Failed reads retain the prior position. See [M5 main.c](../restart_highscore_m5/main.c), especially lines 43–46, 336–367, 752–756, 766–865, 887–898, and 1011–1029.

The [M6 tree](tilt_tree.c) classifies `delta <= -151` as `LEFT_TILT`, `-150..150` as `NEUTRAL`, and `delta >= 151` as `RIGHT_TILT`. [M7 main.c](main.c) uses that class for the neutral hold and movement direction while retaining M5's step magnitude, clamping, sensor-error handling, automatic firing, game logic, SW2 restart, and flash journal. For every integer delta representable from signed 16-bit raw and neutral values, the tree's direction and neutral rules equal M5's threshold rules. **M7 changes code structure, not intended ship behavior.** The M6 examples were generated from the threshold rule, so this is not evidence of learned improvement or real-sensor accuracy. No flick-to-fire or four-command recognition is present.

## Build and software evidence

`build.py` is an isolated M5 build-script copy. It retains M5's verified ARMCC 5.03 flags, startup/system paths, scatter layout, vector checksum reference, Intel HEX checks, and reserved high-score sectors. It adds `tilt_tree.c` to compilation/linking and rejects a tree copy that differs from `ml/tilt_tree.c` or `.h`. The tree compiled under ARMCC 5 `--c90`; M7 main retains M5's `--c99` setting. Host GCC C89 success alone would not establish ARMCC compatibility; this build gives that separate compiler check. The tree adds no embedded library dependency. No timing, RAM, power, or board behavior was measured.

From the repository root, use Python 3 and host GCC for the host tests. An **optional** rebuild requires ARMCC 5 and the LPC17xx support paths specified in [build.py](build.py); always choose a fresh M7-specific output directory:

```powershell
$env:PYTHONDONTWRITEBYTECODE='1'
python -m unittest discover -s firmware/tests/decision_tree_m7 -p test_host.py -v
python firmware/tests/decision_tree_m7/build.py --output-dir firmware/build/2026-10-29/decision_tree_m7_rebuild_1
```

The first reviewed build is [decision_tree_m7_lpc1768.hex](../../build/2026-10-29/decision_tree_m7_first/decision_tree_m7_lpc1768.hex), **24,600 bytes**, SHA-256 `CE5385387F5D0A8EEF2E1027EB0236466BFCDE5A49076DD842467E1B82DBA416`. Its [build log](../../build/2026-10-29/decision_tree_m7_first/build.log) records ARMCC 5.03 Build 24 compile, assembly, link, FromELF and vector patch with no reported warnings or errors. HEX validation found 8,928 unique data bytes at `0x00000000–0x000022DF`; the reserved `0x0000E000–0x0000FFFF` score sectors contain no HEX data. These checks establish artifact structure, not which Flash Magic erase mode will be selected or whether an existing high score survives programming.

The host test compiles the **actual extracted M5 and M7 `next_ship_x` functions** plus M7's tree with GCC `-std=c89 -pedantic-errors -Wall -Wextra -Werror`. Three tests passed: tree class boundaries, movement/clamp examples, and exact M5/M7 position parity for all `delta=-65535..65535` at five representative ship positions (8, 9, 55, 102, 103). This is software equivalence for those inputs; it does not verify I²C readings, orientation, visual response, or MCU timing.

## Owner report and physical evidence still needed

The owner reports that the M7 HEX was flashed to the LPC1768 and that gameplay looked the same as M5. Treat this as **one qualitative owner-reported pass**. The repository does not contain the Flash Magic selected-file/verify log, board readback, date/operator, photos, trial counts or a detailed record of neutral/left/right, automatic fire, restart and high-score observations for M7. Do not infer measured latency, memory, power, real-sensor classification accuracy or score retention through firmware reprogramming from this report.

If documenting that trial or repeating it, record the following before claiming a stronger physical result:

1. Preserve the **exact HEX that previously worked on this board**, plus its path/hash and programming settings, as the rollback image. The repository's final M5 candidate is [here](../../build/2026-10-29/restart_highscore_m5_final/restart_highscore_m5_lpc1768.hex), but the physically tested M5 HEX has never been identified, so do not assume that candidate is the known working one.
2. Record board/module identity, the existing wiring and supply arrangement, and the selected Flash Magic erase/verify settings. Review the [M5 connection and electrical evidence](../../../manual/hardware-connections.md#current-m5-signal-map-for-the-ieee-report). Record the current best score if its retention matters; a HEX below `0xE000` does not prove Flash Magic preserves score sectors.
3. Record which M7 HEX path/hash Flash Magic selected and its verify result; for a repeat trial, program only the M7 candidate after checking the existing wiring and supply. Reset while holding the sensor in the intended neutral orientation. Stop on a programming or calibration error; do not infer success from the software build.
4. Observe one startup reaching `READY`, neutral hold, deliberate left tilt, return to neutral, deliberate right tilt, automatic shots, and continued gameplay. If practical, reach `GAME_OVER` and verify SW2 restart and best-score display. Record direction, any drift, read errors, frozen movement, unexpected step size, screen behavior, and photos/video. A single successful pass is a limited smoke test, not a reliability or accuracy measurement.
5. If the image behaves incorrectly, reprogram the preserved owner-verified M5 HEX using its recorded settings and verify baseline behavior. Record whether the high score changed or was erased; do not claim reprogramming retention without a before/after sector readback.

No board action was performed by Codex. The current physical observation comes from the owner.
