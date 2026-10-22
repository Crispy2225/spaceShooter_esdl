# Proposed architecture

**Status: PROPOSED / UNVERIFIED.** No firmware or hardware configuration has been inspected. Every module boundary, interface, and hardware assumption below requires confirmation against the real source and setup. No pins, driver names, sample rates, peripheral settings, or memory budgets are assigned here.

```text
MPU6050 acquisition → preprocessing/calibration → shared features
    → selected detector (threshold baseline or decision tree)
    → normalized control events → game logic → ST7735 TFT rendering
```

| Proposed responsibility | Proposed interface or output |
| --- | --- |
| Sensor acquisition | Read time-ordered accelerometer and gyroscope samples from the MPU6050 over I2C; report read errors. |
| Preprocessing and calibration | Apply an agreed neutral reference and consistent filtering or scaling; provide valid, time-associated samples. |
| Shared feature extraction | Derive one documented feature set from the same sensor stream for either detector. Feature definitions and timing remain open. |
| Selected detector | Map features to left, right, fire, or no-command, with any confidence or debounce behavior defined during implementation. |
| Control normalization | Convert detector output to a single control-event contract, including neutral/no-command and fire-event semantics. |
| Game logic | Consume that contract to update player, enemies, collisions, score, and game state. Actual game behavior remains to be verified. |
| TFT rendering | Draw the resulting game state on the ST7735 over SPI. Driver and update strategy remain unknown. |

**Proposed control contract:** Each detector outputs one of four mutually exclusive classes. Left or right steers only while that class is active; fire and no-command stop steering. Entering fire creates at most one shot event if the cooldown has elapsed; a sustained fire classification does not repeat shots. Both detectors use this same downstream logic.

The threshold baseline and the offline-trained decision tree should receive the **same calibrated features** and produce the **same normalized control events**. This isolates detector choice from game logic and rendering, enabling a fair comparison. The tree would be exported as compact C for MCU inference; its feasibility and resource use remain unmeasured.

**OPEN QUESTIONS:** What firmware already exists? What sensor orientation and gestures does the physical controller support? What are the actual board, module, display, wiring, startup/calibration, timing, and memory constraints? The website's instructions are a design reference, not a hardware test result.
