# Research plan

**Status: PROPOSED.** Novelty and publication suitability are **NOT YET ESTABLISHED**. This plan contains no measured results or achieved targets.

## Question and possible contribution

**Candidate question:** For motion control of an LPC1768 space shooter, how does a threshold-based detector compare with a compact, offline-trained decision tree exported to C, given identical sensor features and game control handling?

**Candidate gap:** A small, reproducible, on-device comparison of detection quality and MCU costs for this specific gesture set may be useful. A related-work review must establish whether comparable studies already exist before claiming any gap or novelty.

**Potential contribution:** A documented gesture dataset and protocol, an interpretable threshold baseline, a compact tree, and measured tradeoffs between detection quality, latency, and memory. Dataset collection, release rights, feasibility, and contribution remain open.

## Comparison design

- Define labels **left**, **right**, **fire**, and **no-command**. Include deliberate gestures, neutral holding, transitions, and incidental movements that should not issue commands. Record label timing and ambiguity rules before collection.
- Capture time-associated raw recordings with session, participant (if applicable), device orientation, and calibration context. Decide sampling and feature windows from the verified hardware and firmware.
- Use the same preprocessing, calibration, feature definitions, and downstream control-event handling for both detectors. Tune the threshold baseline using training data only; train and select the decision tree offline using training data only.
- Hold out complete recordings or sessions for final testing where possible; keep adjacent windows from one recording in the same split. Prefer participant-held-out evaluation if enough participants exist. Fix the split before final model selection, document class balance, and avoid tuning on the final test set.
- Measure macro-F1 and per-class precision/recall/F1, false activations during no-command or incidental movement, missed gestures, inference latency on the MCU, and flash and RAM usage. Define the counting windows and measurement method before reporting results.
- Compare errors and resource costs under the same test conditions. Report uncertainty or variability across sessions where the sample size permits, plus dataset and hardware limitations.

## Related work and limits

Review primary research and relevant embedded gesture-recognition work for comparable sensors, gesture sets, classifiers, datasets, evaluation splits, and MCU resource measurements. Record complete, verified citations and explain how this study differs. Do not invent papers or citation details.

Likely limits include a small or user-specific dataset, mounting/orientation effects, label ambiguity, and performance that may not generalize beyond this board and gesture set. Actual hardware, firmware, dataset availability, and publication prospects remain unverified.
