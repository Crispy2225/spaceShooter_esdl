# Project context

**Project title:** Flick & Fly: Motion-Controlled Space Shooter.

**Current demonstration:** The LPC1768 [M5 firmware](firmware/tests/restart_highscore_m5/README.md) implements MPU6050 startup calibration and tilt steering, TFT gameplay with automatic firing, enemies, collisions, score/lives, `GAME_OVER`, SW2 restart, and a flash-backed best score. The owner reports successful board operation and power-cycle score retention in the [M5 physical verification record](firmware/tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md). The exact physically tested M5 HEX, SW2 voltages, and module electrical details remain unidentified or unmeasured. The [current-state and evidence index](docs/CURRENT_STATE_AND_EVIDENCE_2026-10-29.md) separates source, build, and physical evidence.

**Candidate research question (PROPOSED, not a completed result):** On the LPC1768, how does threshold-based gesture detection compare with a compact decision tree trained offline and deployed as C for left, right, fire, and no-command control? The [original synopsis](docs/Updated_ESDL_Synopsis_ML_IMU_Gesture_Recognition.docx), [research plan](docs/RESEARCH_PLAN.md), and [proposed architecture](docs/ARCHITECTURE.md) preserve that study. No completed classifier comparison or deployed tree is evidenced in the inspected M5 repository. The game currently fires automatically; it does not implement flick-to-fire or adaptive difficulty.

## Current state

**SOURCE/BUILD EVIDENCE:** The repository contains isolated M1–M5 firmware, build scripts, and HEX artifacts. M5 source implements the game functions above; its README records the build checks. The illustrated [player guide](index.html) is not a live sensor connection.

**OWNER-REPORTED PHYSICAL EVIDENCE:** M4.1 calibration and gameplay remained functional; M5 SW2 restart worked; a saved high score survived switching board power off and on. The [physical record](firmware/tests/restart_highscore_m5/PHYSICAL_VERIFICATION.md) gives the evidence boundary. The exact flashed M5 HEX, SW2 electrical levels, fitted module 5 V/signal-level compatibility, and score retention through firmware reprogramming remain unverified.

## Project documents

- [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) — scope and current state.
- [docs/CURRENT_STATE_AND_EVIDENCE_2026-10-29.md](docs/CURRENT_STATE_AND_EVIDENCE_2026-10-29.md) — dated current-state and evidence index.
- [manual/hardware-connections.md](manual/hardware-connections.md#current-m5-signal-map-for-the-ieee-report) — M5 source/manual signal map and unresolved physical routes.
- [docs/ENGINEERING_PLAYBOOK.md](docs/ENGINEERING_PLAYBOOK.md) — permanent working rules.
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — proposed system flow.
- [docs/RESEARCH_PLAN.md](docs/RESEARCH_PLAN.md) — candidate comparison study.
- [docs/DECISIONS.md](docs/DECISIONS.md) — constraints, proposals, and open questions.
- [docs/STATUS_AND_HANDOFF.md](docs/STATUS_AND_HANDOFF.md) — evidence, blockers, and session handoff.

**Report preparation:** Define whether the final IEEE paper reports the completed prototype alone or includes a separately evidenced detector comparison. Gather the missing measurements and image identity listed in the [evidence index](docs/CURRENT_STATE_AND_EVIDENCE_2026-10-29.md).

**Historical status:** A 22 October 2026 read-only inventory described only a proposal-stage website, no embedded source then identified, and an earlier public `main` state. Later milestone records supersede that *status*. Preserve the original proposal and milestone records; see the [engineering playbook](docs/ENGINEERING_PLAYBOOK.md).
