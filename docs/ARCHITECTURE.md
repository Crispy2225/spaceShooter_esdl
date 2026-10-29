# Proposed architecture

**Original proposal status: PROPOSED / UNVERIFIED.** At the time this architecture was drafted, no firmware or hardware configuration had been inspected. Its module boundaries and four-command interface remain proposed. No memory budget or performance measurement is established here.

**Later M6 note:** The [offline proof of concept](../ml/README.md) confirms one M5-compatible feature definition from source inspection: calibrated raw X delta. It trains and exports a three-class tree from generated examples and compares it with a threshold baseline on a generated holdout. The broader four-command control flow below remains a proposal and is not deployed in M5.

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
