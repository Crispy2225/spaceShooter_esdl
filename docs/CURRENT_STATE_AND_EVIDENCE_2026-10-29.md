# Flick & Fly: current state and evidence — 29 October 2026

This index is the current entry point for preparing the Embedded Systems Lab IEEE report. It separates what the [M5 source](../firmware/tests/restart_highscore_m5/main.c) implements, what the [build records](../firmware/tests/restart_highscore_m5/README.md) check, and what the owner reports observing on the LPC1768 trainer. Earlier proposals and milestone reports remain dated records, not replacements for current evidence. The owner identifies `83a9468` (`Add M5 restart and persistent high score`) as the pushed M5 checkpoint; this index does not independently verify repository history.

## Purpose and implemented demonstration

Flick & Fly is an LPC1768 space-shooter demonstration using an MPU6050 and a 128 × 160 TFT. The isolated [M5 implementation](../firmware/tests/restart_highscore_m5/main.c) retains M4.1 startup calibration and tilt steering, fires automatically, moves enemies, handles projectile/enemy and ship/enemy collisions, tracks score and lives, and displays `GAME_OVER` with final and best scores. After a stable release, a debounced SW2 press restarts from `GAME_OVER` without repeating successful calibration. A new best score is written to the reserved internal-flash journal when a completed game exceeds the previous best. See the [M5 README](../firmware/tests/restart_highscore_m5/README.md) for behavior and storage design.

**The game demonstration is not the proposed classifier study.** The [original synopsis](Updated_ESDL_Synopsis_ML_IMU_Gesture_Recognition.docx), [proposed architecture](ARCHITECTURE.md), and [research plan](RESEARCH_PLAN.md) describe a tuned threshold detector versus an offline-trained shallow decision tree for left, right, fire, and no-command gestures. The inspected M5 game does not implement flick-to-fire or a deployed decision tree, and the repository does not contain a completed labeled comparison experiment. Those documents remain proposals; do not report their expected outcomes as results. M5 also uses fixed enemy timing rather than adaptive difficulty.

## Milestone trail

The dated [M0 baseline audit](../firmware/M0_BASELINE_AUDIT_2026-10-27.md), [M1 build recovery](../firmware/M1_BUILD_RECOVERY_2026-10-27.md), and [validation record](../firmware/VALIDATION_2026-10-27.md) document early work. Isolated test records then cover [M1 MPU calibration](../firmware/tests/mpu6050_tilt_calibration/README.md), [M2 TFT rendering](../firmware/tests/tft_renderer_m2/README.md), [M3 tilt and shooting](../firmware/tests/tilt_shooter_m3/README.md), [M4 gameplay](../firmware/tests/core_gameplay_m4/README.md), [M4.1 calibration and visual polish](../firmware/tests/core_gameplay_m4_1/README.md), and [M5 restart and best score](../firmware/tests/restart_highscore_m5/README.md). A statement such as “not yet flashed” in an earlier README records its status when written; later physical observations are recorded separately. The [combined-test connection record](../manual/hardware-connections.md) and [early electrical readiness review](../manual/hardware-readiness.md) are also historical hardware references.

## Evidence boundary

| Category | What is supported | Limit |
| --- | --- | --- |
| Source inspection | [M5 `main.c`](../firmware/tests/restart_highscore_m5/main.c) contains calibration, game logic, automatic firing, SW2 input/debounce, `GAME_OVER` restart, and the two-sector high-score journal. [`memory.sct`](../firmware/tests/restart_highscore_m5/memory.sct) reserves flash sectors 14–15; [`build.py`](../firmware/tests/restart_highscore_m5/build.py) checks the HEX address boundary and vector checksum. | Source review establishes intended behavior, not board-level reliability or measured timing. |
| Build and HEX | The [M5 README](../firmware/tests/restart_highscore_m5/README.md) and [physical record](../firmware/tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md) document ARMCC 5.03 Build 24 success, no reported compiler warnings/errors, final HEX checksum/structure checks, and a final image with 8,876 unique data bytes at `0x00000000–0x000022AB`. | Build checks do not identify which image was physically flashed or establish FPS, latency, power, or module electrical safety. |
| Owner-reported physical verification | The [M5 physical verification record](../firmware/tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md) reports working M4.1 calibration/gameplay, SW2 restart from `GAME_OVER`, a high-score update, and score retention after switching board power off and back on. The owner previously reported 10/10 M4.1 cold starts/resets reaching `READY`; the M4.1 HEX identity appears in its [README](../firmware/tests/core_gameplay_m4_1/README.md). | M5 trial counts, measured score values, photos, programming log, and selected HEX were not supplied. These are owner reports, not independently archived measurements. |
| Unverified or unmeasured | SW2 voltages/continuity, exact module 5 V input and I/O compatibility, journal interruption/fault cases, firmware-reprogramming score retention, and quantitative game or detector performance. | Do not promote these to successful tests or results. |

### M5 image identity

Two distinct candidate artifacts exist:

| Candidate | Size | SHA-256 |
| --- | ---: | --- |
| [First M5 HEX](../firmware/build/2026-10-29/restart_highscore_m5_first/restart_highscore_m5_lpc1768.hex) | 24,320 bytes | `3947A0D4C366D40C262B6ABA388C67B8C7D861DF18CE2F3D367FC0675BCA211A` |
| [Final reviewed M5 HEX](../firmware/build/2026-10-29/restart_highscore_m5_final/restart_highscore_m5_lpc1768.hex) | 24,460 bytes | `2B6E85CFED81B804C98D59F0245B7F63D59C8437CA0A06AA66373C435D276854` |

**The physically tested M5 HEX remains unidentified.** Neither candidate can be assigned to the board test without the selected-file path/hash, Flash Magic log, or board readback. The final candidate's structural checks are software evidence only; see [M5 physical verification](../firmware/tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md).

## Hardware and persistence limits

The owner reports a CNB-to-CNB1 FRC cable and active-low SW2 on P2.12. The [M5 README](../firmware/tests/restart_highscore_m5/README.md) records that contact continuity, released/pressed voltage, and the external pull circuit were not measured; the source configures an internal pull-up. Trainer connector mappings are documentary candidates unless tied to the fitted cable and measured pin. The owner reports supplying the MPU6050 and TFT modules from 5 V, but the exact module variants, 5 V input compatibility, pull-up rails, and MCU-facing signal levels remain unresolved. Consult the [hardware connections record](../manual/hardware-connections.md), [early readiness review](../manual/hardware-readiness.md), and [M5 physical record](../firmware/tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md) without treating a chip-level specification as proof of breakout-board safety.

The owner reports that a saved high score survived a **board power-off/power-on cycle**. This is distinct from retention through a later firmware programming operation. “Erase Blocks Used by Hex File” was reported as a Flash Magic setting, but no score-sector readback before and after reprogramming was documented. **Firmware-reprogramming preservation remains unverified.**

## Evidence still needed for the IEEE paper

- Confirm report scope and course requirements. If the proposed detector comparison is claimed, preserve a research question, labeled timestamped recordings, label rules, participants/sessions or an explicit single-user scope, fixed data splits, equal-feature baseline/tree procedures, and reproducible predictions and metrics. Otherwise present the completed game as a prototype and the comparison as future work.
- Record the as-built board/module revisions, connector and jumper state, pin/rail mapping, and measured SW2 and module-facing voltage levels.
- Tie physical tests to the exact flashed M5 image and programming settings; retain the log or board readback, test procedure, date/operator, counts, numerical scores, failures, and photos/video where available.
- Measure any reported timing, memory, or power result with units, method, repeated trials, raw data, and a reproducible calculation. The current “smooth” observation is qualitative, and a nominal 40 ms frame interval is not a measured frame rate.
- Test and document journal fault cases and Flash Magic reprogramming retention before claiming either property physically verified.
- Prepare an as-built architecture figure, results tables from preserved data, limitations, and a verified IEEE bibliography. The [synopsis](Updated_ESDL_Synopsis_ML_IMU_Gesture_Recognition.docx) contains initial references; conference papers and component manuals must be classified separately from journal articles and checked against the course rubric.
