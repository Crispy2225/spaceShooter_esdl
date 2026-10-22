# Project context

**Proposed title:** Flick & Fly: Motion-Controlled Space Shooter.

**Purpose (PROPOSED):** Build and study an Embedded Systems Lab space-shooter demonstration controlled by motion. The intended hardware is an LPC1768 MCU, an MPU6050 connected over I2C, and an ST7735 TFT connected over SPI. The team reports access to these parts; exact variants, connections, and operation still require physical verification.

**Candidate research question (PROPOSED):** On the LPC1768, how does threshold-based gesture detection compare with a compact decision tree trained offline and deployed as C for left, right, fire, and no-command control?

## Current state

**VERIFIED locally:** This folder contains a player-guide website: [README.md](README.md), `index.html`, `styles.css`, and `app.js`. The guide describes proposed tilt steering, a downward wrist flick for firing, and a physical start/restart button; its illustrations are not live sensor readings. No identifiable LPC1768 firmware, embedded build files, or hardware drivers were found in the inspected local folder. At the read-only check on 2026-10-22, tracked local files matched public `main` at `f81d0d9499d807a26296995c0f123b1b7aca9f72`.

**UNVERIFIED:** Firmware availability, the toolchain and programmer, exact board and module variants, wiring, sensor orientation, display setup, flashing, and working hardware behavior. The website's description does not establish that the physical system currently works.

## Project documents

- [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) — scope and current state.
- [docs/ENGINEERING_PLAYBOOK.md](docs/ENGINEERING_PLAYBOOK.md) — permanent working rules.
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — proposed system flow.
- [docs/RESEARCH_PLAN.md](docs/RESEARCH_PLAN.md) — candidate comparison study.
- [docs/DECISIONS.md](docs/DECISIONS.md) — constraints, proposals, and open questions.
- [docs/STATUS_AND_HANDOFF.md](docs/STATUS_AND_HANDOFF.md) — evidence, blockers, and session handoff.

**Immediate next task:** Obtain professor approval, then verify the board, modules, wiring, toolchain, flashing, and hardware functionality before implementation.

**Critical blockers:** No embedded source or build path is identified in this repository; hardware configuration, programming path, and operation remain unverified. Git mutations remain under the user's control; see the [engineering playbook](docs/ENGINEERING_PLAYBOOK.md).
