# Research plan beyond the Flick & Fly prototype

## Current evidence and question

[M6](../ml/README.md) demonstrates an end-to-end generated-data pipeline for one calibrated X-axis feature and three labels: `LEFT_TILT`, `NEUTRAL`, `RIGHT_TILT`. Its depth-two tree and threshold baseline agree perfectly on held-out **generated** examples because the labels were generated from that threshold rule. [M7](../firmware/tests/decision_tree_m7/README.md) compiles the tree into a separate LPC1768 firmware image; the owner reports that the flashed game appears to behave like M5. Neither result establishes an accuracy gain, robustness gain, or measured on-device cost.

**Research question:** Given the same recorded MPU6050 features and downstream game control, can a compact decision tree reduce false steering commands or missed deliberate tilts compared with a threshold baseline enough to justify its extra implementation cost on the LPC1768? Novelty and publication suitability require a verified related-work review and are not claimed here.

## Next experiment

1. Record and label actual MPU6050 streams for neutral holding, left tilt and right tilt. Save timestamps, raw axes, calibration offset, device orientation, session ID, label rules and any ambiguous transitions. Obtain appropriate permission before sharing recordings.
2. Freeze train/test membership by **recording or session**, keeping adjacent samples from one recording together. If the sample size permits, hold out participants as well. Fit all preprocessing, thresholds and tree choices on training data only.
3. Compare the tree and a threshold baseline on identical features and the same held-out recordings. Report per-class precision/recall/F1, macro F1, false activations during neutral/incidental motion and missed deliberate tilts, with denominators, counts and uncertainty where possible. Preserve predictions and the calculation script.
4. Verify Python/C prediction parity on recorded features. If an on-device trial is approved, measure inference latency, code/data memory and power with a written method, units and repeated trials. Keep the M5 image and programming settings for rollback; distinguish build checks from observed board behavior.
5. Only after a real three-class baseline is established, investigate multiple axes, temporal windows, mounting variation and additional gestures. A deliberate fire gesture would require a new labeled class and an explicit one-shot event/cooldown design. Do not describe it as a current control.

The [synopsis](Updated_ESDL_Synopsis_ML_IMU_Gesture_Recognition.docx) remains a historical four-command proposal, not a record of completed experiments. Review primary papers and verified component sources before asserting a literature gap; do not infer it from the current prototype alone.
