# Decision log

This log separates accepted constraints from proposals. A proposal becomes an accepted decision only after the user explicitly approves it; record the date and rationale then.

## Accepted constraints

| Item | Basis |
| --- | --- |
| The user alone performs every Git operation; Codex does not check Git status. | Explicit project instruction; permanent rule in [ENGINEERING_PLAYBOOK.md](ENGINEERING_PLAYBOOK.md). |
| Preserve the existing player-guide website and its README. | Explicit documentation-only scope. |
| This phase creates only the six requested Markdown documents and the needed `docs` directory. | Explicit current-task scope. |
| Do not assume firmware, hardware behavior, or research novelty has been verified. | Audit and explicit project instruction. |

## Verified local observations

- `README.md`, `index.html`, `styles.css`, and `app.js` form a local player-guide website. The README calls it instructional and says its animations are illustrative.
- No identifiable LPC1768 firmware, embedded build files, or hardware drivers were found in the inspected local folder. The remote repository has not been compared with it.
- Actual firmware source and hardware configuration remain **UNVERIFIED**.

## Proposed decisions, awaiting approval

| Proposal | Reason to consider | Status |
| --- | --- | --- |
| Use the flow and shared detector interface in [ARCHITECTURE.md](ARCHITECTURE.md). | Enables a consistent detector comparison. | PROPOSED; not approved. |
| Compare a tuned threshold baseline with an offline-trained compact decision tree. | Tests quality and resource tradeoffs. | PROPOSED; not approved. |
| Use the candidate protocol and metrics in [RESEARCH_PLAN.md](RESEARCH_PLAN.md). | Supports a reproducible evaluation. | PROPOSED; not approved. |

## Open questions

- Where is the actual firmware, and what is its current toolchain and build procedure?
- Which LPC1768 board, MPU6050 and ST7735 modules, wiring, voltages, and sensor orientation are used?
- What hardware functions have been demonstrated, by whom, and with what evidence?
- Is the proposed gesture set and detector comparison feasible and appropriate after source and hardware review?
- What does the related work establish about novelty?
