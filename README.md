# Flick & Fly — LPC1768 space shooter

Flick & Fly is an Embedded Systems Lab prototype built around an LPC1768, MPU6050 accelerometer and 128 × 160 TFT. Startup calibration establishes a neutral hand position. Tilting steers the ship while shots fire automatically; enemies, collisions, score, lives and `GAME_OVER` form the game loop. The [M5 firmware](firmware/tests/restart_highscore_m5/README.md) adds SW2 restart and a flash-backed best score. The owner reports these M5 functions working on the trainer and a best score surviving a board power cycle; the exact M5 HEX used in that test is unidentified.

## Decision-tree milestone

[M6](ml/README.md) implements a reproducible **offline** three-class workflow: generate labeled illustrative X-axis deltas, train a depth-two scikit-learn decision tree, evaluate on a fixed generated holdout, compare a threshold baseline, export C and check host C parity. The 1,201 values are generated examples, **not MPU6050 recordings**. On the 175 generated holdout rows, tree and baseline each scored 1.000 accuracy and macro F1 with identical predictions. Because the labels come from the threshold rule, this shows that the pipeline reproduces that rule; it does not show better real-world recognition.

[M7](firmware/tests/decision_tree_m7/README.md) integrates the exported tree into a **separate M5-derived image** for neutral and left/right decisions. It keeps M5's tilt-magnitude step size and game behavior. Three host tests passed, and the isolated ARMCC 5.03 Build 24 build produced a structurally checked [M7 HEX](firmware/build/2026-10-29/decision_tree_m7_first/decision_tree_m7_lpc1768.hex). The owner now reports flashing M7 and seeing gameplay that appears the same as M5. That is an **owner-reported physical observation**, not an instrumented test or measured classifier result. M4.1 and M5 remain preserved.

## Reproduce the software checks

From the repository root, use Python 3.12, the pinned dependency in [ml/requirements.txt](ml/requirements.txt), and host GCC for C tests:

```powershell
python -m pip install -r ml/requirements.txt
python ml/run_all.py
python -m unittest discover -s firmware/tests/decision_tree_m7 -p test_host.py -v
```

`ml/run_all.py` generates the CSV, trains/evaluates the model, exports C, refreshes the M6 result blocks in the [player guide](index.html) and M6 README, and runs five host tests. The M7 command compares its extracted steering function with M5 across the possible signed raw-minus-neutral delta range at representative ship positions. An **optional** M7 rebuild requires the installed ARMCC 5 toolchain and the local support files expected by [M7 build.py](firmware/tests/decision_tree_m7/build.py); choose a fresh `firmware/build/.../decision_tree_m7_*` output directory. The existing M7 HEX does not need rebuilding for a documentation review.

## Evidence and submission

The player guide is an illustrated explanation, not a live sensor connection. [Architecture](docs/ARCHITECTURE.md), the [current evidence index](docs/CURRENT_STATE_AND_EVIDENCE_2026-10-29.md), the [hardware connection record](manual/hardware-connections.md) and the [final submission audit](docs/FINAL_SUBMISSION_AUDIT.md) distinguish software checks, owner observations and unresolved details. The [historical synopsis](docs/Updated_ESDL_Synopsis_ML_IMU_Gesture_Recognition.docx) proposes a larger four-command study; flick-to-fire, real-recording evaluation, adaptive difficulty and measured MCU latency/memory/power are **not completed results**.

For research beyond this prototype, collect labeled MPU6050 sessions, hold out complete recordings, compare tree and threshold control on the same real features, then measure error rates and MCU cost. Before submission, identify the exact flashed M7 file and preserve any programming log or board photo available. See the [audit checklist](docs/FINAL_SUBMISSION_AUDIT.md) for the remaining manual decisions, including which generated build artifacts to include.
