# Engineering playbook

## Absolute Git ownership rule

The user alone performs **all Git operations**, permanently, in every phase and session. Codex must never execute Git commands, invoke Git through an IDE or extension, or use another mechanism to perform Git operations. This includes clone, pull, fetch, commit, push, all branch operations, index changes, resets, checkout, restore, stash, merges, rebases, cherry-picks, reverts, cleanup, remote changes, and repository-history changes. Codex must not check Git status. If repository status or history is needed, ask the user to run the relevant command and provide its output. Treat a remote comparison as unverified until the user supplies its evidence.

## Working sequence

1. **Audit:** Read the relevant local files and record what is verified, proposed, unverified, and unanswered. Preserve existing work, including the player-guide website.
2. **Proposal:** State the smallest change, expected behavior, dependencies, risks, and verification plan.
3. **Explicit user approval:** Obtain approval for the proposal before implementation. Ask before expanding scope or changing an accepted decision.
4. **Implementation:** Make minimal, targeted changes. Do not refactor speculatively or overwrite work outside the approved scope.
5. **Testing:** Run the applicable build and tests when the toolchain and hardware are available. Report exact commands, environment, results, and anything not tested; never claim success from a missing build or simulated evidence.
6. **Review:** Summarize changed files, evidence, limitations, risks, and questions for the user. The user handles every Git operation.

Work locally on the files the user has authorized. Do not infer that the player guide is firmware or that its described hardware behavior is verified. If required source or equipment is missing, document the blocker and stop dependent implementation.

## Hardware safety and evidence

Before powering or flashing hardware, verify the actual board and module identities, supply and logic voltages, wiring, pin assignments, display and sensor interfaces, and toolchain procedure from the physical setup and authoritative documentation. Obtain the user's approval for a proposed hardware test, use a safe test plan with observable pass/fail criteria, and avoid changing connections or flashing unverified binaries. Record observed behavior separately from assumptions and website illustrations.

## Documentation and handoff

Update the context, architecture, research plan, decision log, and [status/handoff](STATUS_AND_HANDOFF.md) only when their facts or decisions change. Label evidence and proposals honestly, link supporting local files or user-provided observations, and record approvals before treating a proposal as accepted. End each session with changed files, tests and results, risks, pending approval, and the next task. Do not fabricate citations, measurements, milestones, or hardware settings.
