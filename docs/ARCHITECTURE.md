# Flick & Fly architecture

## Implemented game paths

```text
MPU6050 acceleration over I²C2
        ↓ signed raw X, Y, Z; successful reads only
32-sample startup neutral calibration
        ↓ delta = (raw X − neutral X) × MOVE_SIGN (+1)
M5: inclusive ±150-count threshold  |  M7: exported three-class tree
        ↓ neutral / left / right decision
M5 step magnitude (1–5 pixels), ship bounds and game state
        ↓ automatic shots, enemies, collisions, score, SW2 restart
128 × 160 TFT over SPI0; best score journal in reserved flash sectors
```

[M5 `main.c`](../firmware/tests/restart_highscore_m5/main.c) defines `MOVE_AXIS=0`, `MOVE_SIGN=+1` and `DEAD_ZONE=150` (lines 43–46), averages valid calibration reads (lines 766–865), computes the delta after a successful gameplay read (lines 1011–1029) and uses the threshold plus tilt magnitude to move the ship (lines 887–898). A failed read leaves the current ship position in place. The M5 source retains its original behavior and owner-reported physical evidence.

[M7 `main.c`](../firmware/tests/decision_tree_m7/main.c) changes only the neutral/direction decision in that steering function. It calls the [M6 exported tree](../firmware/tests/decision_tree_m7/tilt_tree.c): `delta <= -151` → `LEFT_TILT`, `-150..150` → `NEUTRAL`, `delta >= 151` → `RIGHT_TILT`. M7 retains the same step-size calculation, clamping, game input path, automatic firing and persistence code. Host tests found position parity between M5 and M7 for all `delta=-65535..65535` at five representative ship positions. The tree compiled in ARMCC 5 C90 mode and linked into the separate M7 HEX. The owner reports that M7 was flashed and gameplay looked the same as M5; no instrumented board comparison was supplied.

## Offline ML path and limits

```text
Deterministic generated X-delta values and threshold-generated labels
    → fixed train/holdout split → depth-two scikit-learn tree
    → generated-data comparison with the same threshold baseline
    → readable C export → host parity tests → isolated M7 firmware build
```

The [M6 dataset and report](../ml/README.md) establish software reproducibility. They are not sensor recordings, and perfect agreement with their own threshold-derived labels does not estimate real-world recognition accuracy. This implementation uses one instantaneous calibrated accelerometer axis. It has no gyroscope feature, temporal window, fire class, flick-to-fire event or adaptive difficulty.

The [original synopsis](Updated_ESDL_Synopsis_ML_IMU_Gesture_Recognition.docx) proposed a broader four-command study and multi-axis/temporal features. Those remain future research. The [research plan](RESEARCH_PLAN.md) describes the recordings, held-out sessions and MCU measurements needed to evaluate them. Actual module wiring and electrical limits are separated in the [hardware connection record](../manual/hardware-connections.md).
