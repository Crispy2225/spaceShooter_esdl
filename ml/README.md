# M6: offline tilt decision-tree proof of concept

This is a **synthetic-data demonstration**, not a trained sensor model or a deployed game feature. No labeled MPU6050 recordings were found in the project. `train_export.py` generates 1,201 illustrative integer X-delta values from -600 through +600 and assigns labels by a written rule: `delta_x <= -150` is `LEFT_TILT`, `delta_x >= 150` is `RIGHT_TILT`, and all values between are `NEUTRAL`. These are deliberately constructed examples, **not measured sensor samples**. The thresholds are teaching choices; they are not learned from physical behavior.

The single input matches the *shape* of M5's steering feature: `delta_x = (long)raw[0] - neutral[0]` with `MOVE_AXIS=0` and `MOVE_SIGN=+1`. M5 obtains signed 16-bit raw acceleration from an I²C2 read and computes `neutral[0]` from 32 successful startup calibration samples. It handles failed reads by retaining the current ship position. Physical orientation and left/right mapping for a new user or mounting have not been validated for this tree. The tree uses no gyro or temporal feature and cannot recognize a flick or a fire command.

The training script fits `DecisionTreeClassifier(max_depth=2, random_state=0)` and exports `tilt_tree.c` as plain nested `if` statements. The checked-in C file was generated with scikit-learn 1.7.2. A host test confirmed exact regenerated text and prediction parity for all 1,201 synthetic inputs. The exported interface is C89 style and uses `long`, as M5 does, but no ARMCC 5 compile or LPC1768 execution was run.

```powershell
# In a Python environment with package access:
python -m pip install -r ml/requirements.txt
python ml/train_export.py
python -m unittest discover -s ml/tests -v
```

On 29 October 2026, the bundled Python 3.12.14, scikit-learn 1.7.2, and host MinGW GCC ran the training/export command above and `python -m unittest discover -s ml/tests -v`: **3 tests passed**. The C test compilation uses `gcc -std=c89 -Wall -Wextra -Werror -shared`; tests cover all 1,201 synthetic values plus explicit boundary and extreme inputs. In an environment without scikit-learn, the two C host tests can still run and the export/parity test reports a skip. Sample inputs and C outputs: `-151` and `-150` → `LEFT_TILT`; `-149`, `0`, and `149` → `NEUTRAL`; `150` and `151` → `RIGHT_TILT`. The learned integer rules are exactly `delta_x <= -150`, then `delta_x <= 149`, else right. This is a parity and implementation check against synthetic labels, **not a real-data accuracy estimate**.

M5 firmware remains untouched. Its existing `next_ship_x` maps tilt *magnitude* to 1–5 pixels per frame after a ±150 count dead zone; replacing it with three classes would remove that behavior. No labeled real data, measured physical sign/mounting mapping, real-world accuracy comparison, ARMCC compatibility build, or board timing/resource measurements support integration yet. A later experiment should collect labeled recordings with calibration and orientation metadata, split by session, train and test on held-out recordings, compare against the existing threshold control on the same feature, verify generated C parity, then assess control behavior and MCU cost before any firmware change.
