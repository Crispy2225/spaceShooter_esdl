# Flick & Fly final submission audit — 29 October 2026

**Review decision:** The local project is ready to show a professor as a **game prototype with a reproducible generated-data ML workflow and an owner-reported M7 board demonstration**. It is not yet evidence for real-world classifier accuracy or a finished four-command research study. The final repository submission still needs the student's artifact selection, Git checkpoint and course-format check.

## Milestones and evidence

| Milestone | Supported result | Evidence limit |
| --- | --- | --- |
| M4.1 / M5 game | [M5 source](../firmware/tests/restart_highscore_m5/main.c) implements calibrated tilt, automatic firing, collisions, scoring, SW2 restart and flash-backed best score. The [M5 physical record](../firmware/tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md) preserves owner reports of working gameplay/restart and power-cycle score retention. | Exact physically tested M5 HEX, trial data, voltages and programming log are absent. |
| M6 offline ML | [Dataset generator](../ml/generate_dataset.py), [CSV](../ml/data/synthetic_tilt.csv), [training/export](../ml/train_export.py), [report](../ml/evaluation.json) and [C](../ml/tilt_tree.c) reproduce a depth-two, three-class tree. Five host tests passed in this closure pass. Generated holdout: 175 rows; tree and baseline each have accuracy and macro F1 of 1.000 and identical predictions. | Labels were generated from the baseline threshold. No measured sensor samples, real-data accuracy or ML superiority. |
| M7 integration | [Isolated M7 source](../firmware/tests/decision_tree_m7/main.c) uses the tree for neutral/direction and retains M5 movement magnitude. Three host tests passed in this closure pass. [Build log](../firmware/build/2026-10-29/decision_tree_m7_first/build.log) records ARMCC 5.03 Build 24 success and a checked [M7 HEX](../firmware/build/2026-10-29/decision_tree_m7_first/decision_tree_m7_lpc1768.hex), SHA-256 `CE5385387F5D0A8EEF2E1027EB0236466BFCDE5A49076DD842467E1B82DBA416`. | The owner reports flashing M7 and seeing gameplay appear like M5. A selected-file/verify log, board readback, detailed physical checklist, photos and measured MCU costs are absent. |

The [player guide](../index.html) explains the game, implemented control path, generated-data pipeline, software evidence and owner-reported M7 status. The [architecture](ARCHITECTURE.md), [research plan](RESEARCH_PLAN.md), [current evidence index](CURRENT_STATE_AND_EVIDENCE_2026-10-29.md), [status handoff](STATUS_AND_HANDOFF.md) and [hardware connection record](../manual/hardware-connections.md) provide the supporting detail. The only `.docx` found is the [original project synopsis](Updated_ESDL_Synopsis_ML_IMU_Gesture_Recognition.docx), written as a proposal; no completed IEEE report draft or submission checklist was found locally.

## Reproduce software evidence

From the repository root, Python 3.12 with the pinned scikit-learn dependency and host GCC can run the offline ML pipeline and host C tests:

```powershell
python -m pip install -r ml/requirements.txt
$env:PYTHONDONTWRITEBYTECODE='1'
python ml/run_all.py
python -m unittest discover -s firmware/tests/decision_tree_m7 -p test_host.py -v
```

`ml/run_all.py` regenerates the dataset, evaluation report and exported C, refreshes bounded M6 result blocks in the M6 README and player guide, and runs five tests. The separate M7 host command runs three tests against extracted M5/M7 steering functions. Host tests and generated-data metrics do not measure board performance.

**Optional M7 build command, not a required documentation or submission step:**

```powershell
python firmware/tests/decision_tree_m7/build.py --output-dir firmware/build/2026-10-29/decision_tree_m7_rebuild_1
```

Use a **fresh** M7-specific output directory. This script expects ARMCC 5 and LPC17xx support under `C:\Keil\ARM` and the preserved 27 October raw/final checksum reference under `firmware/build/`. It refuses to overwrite an existing output. The existing candidate HEX was checked during the earlier M7 build; no firmware rebuild or flash occurred in this documentation pass. A reviewer without that toolchain can reproduce M6 and host tests but cannot reproduce the ARMCC build unchanged.

## Remaining manual checks

- Confirm the professor's required deliverable format and whether the proposal synopsis must be supplemented by a final IEEE report. Do not present its future-tense objectives as completed results.
- Preserve the exact M7 file selected in Flash Magic, its SHA-256 or verify/readback log if available, date/operator, and a short record of neutral/left/right, automatic fire, game-over and SW2 behavior. The current M7 report is qualitative.
- Identify the exact known-working M5 HEX for rollback. The final repository M5 candidate has not been tied to the owner's earlier board test.
- Record fitted MPU/TFT module variants, actual wire endpoints, supply/I/O levels and switch voltages if electrical safety or an as-built wiring claim is required. The [connection map](../manual/hardware-connections.md) is source/manual supported, not a wire-by-wire measurement.
- Keep real sensor collection, recording/session-held-out evaluation, false-trigger analysis and measured MCU latency, code/data memory and power as **future experiments**. Flick-to-fire, four-command recognition and adaptive difficulty are not implemented.
- Review the final Git selection manually. Include the M7 source folder and intended documentation. Decide explicitly whether the final M7 HEX and build log are submission evidence; avoid accidentally adding `.o`, `.axf`, raw HEX and unrelated older build outputs. If linked build artifacts are omitted, adjust repository-facing links or provide an alternate artifact location before publishing. Do not delete, ignore or stage the pre-existing untracked firmware paths automatically.

## Worktree note

Read-only `git status --short` during this audit showed tracked documentation edits, an untracked `firmware/tests/decision_tree_m7/` source folder, an untracked `firmware/build/2026-10-29/decision_tree_m7_first/` output folder, and multiple untracked historical `firmware/build/` and `firmware/tests/` paths that predate this closure pass. The latter are not M7 changes. There is no `.gitignore` in the repository. The student owns staging, artifact inclusion, commit and push. A professor reading the remote repository will only see files the student deliberately includes.

The intended documentation/site edits are `README.md`, `PROJECT_CONTEXT.md`, `docs/ARCHITECTURE.md`, `docs/CURRENT_STATE_AND_EVIDENCE_2026-10-29.md`, `docs/RESEARCH_PLAN.md`, `docs/STATUS_AND_HANDOFF.md`, this audit, `firmware/README.md`, `firmware/tests/decision_tree_m7/README.md`, `ml/README.md`, `index.html`, `styles.css`, `manual/index.html`, `manual/app.js` and `manual/hardware-connections.md`. This pass did not edit M4.1/M5/M7 `.c` source, their build scripts, scatter files, or any historical HEX. The earlier M7 source/build folders are untracked as whole paths; choose their contents deliberately rather than selecting all of `firmware/`.
