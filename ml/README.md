# M6: offline tilt decision-tree proof of concept

This is a **generated-data software demonstration**, separate from the M5 game. No labeled MPU6050 recordings are in this project. The dataset generator creates integer examples from −600 through +600 calibrated X-axis counts and labels them with the M5-style dead zone: less than −150 is `LEFT_TILT`, greater than +150 is `RIGHT_TILT`, and −150 through +150 is `NEUTRAL`. The range is illustrative around the existing threshold, not a claim about observed motion or the full sensor range.

## Feature and model

The one input is `delta_x = ((long)raw[0] - neutral[0]) * MOVE_SIGN`, with M5 `MOVE_AXIS=0` and `MOVE_SIGN=+1`. M5 decodes signed raw acceleration over I²C2 and averages 32 valid calibration samples for `neutral[0]`. Its current steering then converts delta magnitude into 1–5 pixels per frame after a ±150-count dead zone. The M6 tree classifies direction only; it does not preserve that speed behavior, use gyroscope or time-window features, recognize a flick, or issue a fire command.

The separate [M7 firmware](../firmware/tests/decision_tree_m7/README.md) uses the tree for neutral/direction decisions **and keeps M5's magnitude-based step calculation**. Its host tests and ARMCC build passed. The owner now reports that M7 was flashed and gameplay appeared the same as M5; this is a qualitative board report, not a real-data classifier evaluation.

`generate_dataset.py` writes [synthetic_tilt.csv](data/synthetic_tilt.csv) with `delta_x`, `label`, and fixed `train`/`test` split. Values −151, −150, +150, and +151 stay in training so the boundary can be learned. Every seventh value except those boundaries, plus four adjacent examples, is held out. This is a deterministic row split of **generated values from the same labeling rule**, not an independent recording or a generalization test. `train_export.py` fits `DecisionTreeClassifier(max_depth=2, random_state=0)` using only training rows and writes [tilt_tree.c](tilt_tree.c) and [evaluation.json](evaluation.json). The exporter converts half-count tree splits to integer `long` comparisons and emits plain C89-style nested `if` statements. No embedded dependency is added.

## Generated evaluation

<!-- M6_RESULTS_START -->
The generated dataset has **1,201 rows**: **1,026 training** and **175 fixed held-out generated examples**. The depth-2 tree has 3 leaves and split points `-150.5` and `150.5` raw counts. Its integer C rules are `delta_x <= -151` → `LEFT_TILT`, `-150..150` → `NEUTRAL`, and `delta_x >= 151` → `RIGHT_TILT`.

| Generated holdout result | Decision tree | Threshold baseline |
| --- | ---: | ---: |
| Accuracy | 1.000 | 1.000 |
| Macro F1 | 1.000 | 1.000 |
| Prediction agreement | 1.000 | — |

Tree holdout confusion matrix in LEFT/NEUTRAL/RIGHT order: `[[65, 0, 0], [0, 45, 0], [0, 0, 65]]`. The baseline matrix is the same.

| Calibrated X delta | Tree | Baseline |
| ---: | --- | --- |
| `-151` | `LEFT_TILT` | `LEFT_TILT` |
| `-150` | `NEUTRAL` | `NEUTRAL` |
| `0` | `NEUTRAL` | `NEUTRAL` |
| `150` | `NEUTRAL` | `NEUTRAL` |
| `151` | `RIGHT_TILT` | `RIGHT_TILT` |

**Interpretation:** The labels were generated from the threshold baseline rule. Both methods being equivalent on this generated set is expected and does not establish superiority or real-sensor accuracy.
<!-- M6_RESULTS_END -->

## Reproduce

From the repository root, use Python 3.12, a host GCC compatible with the C tests, and the pinned Python dependency:

```powershell
python -m pip install -r ml/requirements.txt
python ml/run_all.py
```

The one command generates the CSV, trains and evaluates the model and threshold baseline on the fixed holdout, exports C, updates the bounded results in this README and the existing [player guide](../index.html), then runs five host tests. The tests compile the generated C with `gcc -std=c89 -Wall -Wextra -Werror -shared`, check class examples and boundaries, compare Python/tree/baseline/C on held-out generated examples, and verify exact regenerated outputs. These M6 host tests do not execute on the LPC1768; M7's separate ARMCC build is documented in its README.

## Evidence limits and next research step

The generated labels use the same rule as the threshold baseline, so equivalent predictions are expected. Neither model has been evaluated against measured sensor data. There is no measured real-world accuracy, board latency, memory, power, or instrumented physical orientation/sign validation for M6/M7. M4.1/M5 firmware and historical milestone records remain unchanged; the M7 integration is isolated.

For an actual comparison, collect labeled, timestamped sensor sessions with calibration and mounting metadata; define transitions and ambiguous cases; split by recording or participant before training; compare the tree and a threshold baseline on the same features and held-out real recordings; verify C parity; then measure on-device behavior and cost before considering any gameplay change. The proposed four-command flick-to-fire study remains future work.
