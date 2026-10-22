# Project context

**Proposed title:** Flick & Fly: Motion-Controlled Space Shooter.

**Purpose (PROPOSED):** Build and study an Embedded Systems Lab space-shooter demonstration controlled by motion. The intended hardware is an LPC1768 MCU, an MPU6050 connected over I2C, and an ST7735 TFT connected over SPI. These parts and connections are project requirements to verify against the actual setup, not a confirmed local implementation.

**Candidate research question (PROPOSED):** On the LPC1768, how does threshold-based gesture detection compare with a compact decision tree trained offline and deployed as C for left, right, fire, and no-command control?

## Current state

**VERIFIED locally:** This folder contains a player-guide website: [README.md](README.md), `index.html`, `styles.css`, and `app.js`. The guide describes tilt steering, a downward wrist flick for firing, and a physical start/restart button; its illustrations are not live sensor readings. No identifiable LPC1768 firmware, embedded build files, or hardware drivers were found in the inspected local folder. The remote repository has not been compared with it.

**UNVERIFIED:** The actual firmware source, toolchain, board variant, modules, wiring, sensor orientation, display setup, and working hardware behavior. The website's description does not establish that the physical system currently works.

## Project documents

- [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) — scope and current state.
- [docs/ENGINEERING_PLAYBOOK.md](docs/ENGINEERING_PLAYBOOK.md) — permanent working rules.
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — proposed system flow.
- [docs/RESEARCH_PLAN.md](docs/RESEARCH_PLAN.md) — candidate comparison study.
- [docs/DECISIONS.md](docs/DECISIONS.md) — constraints, proposals, and open questions.
- [docs/STATUS_AND_HANDOFF.md](docs/STATUS_AND_HANDOFF.md) — evidence, blockers, and session handoff.

**Immediate next task:** Locate and verify the actual firmware source, toolchain, board, modules, wiring, and current hardware functionality before planning implementation.

**Critical blockers:** Firmware location and hardware configuration are unknown. The remote repository has not been compared with this folder. The user alone performs all Git operations; see the [engineering playbook](docs/ENGINEERING_PLAYBOOK.md).
