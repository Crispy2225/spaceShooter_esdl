# Flick & Fly M5 physical verification record

This record separates the owner's physical observations from repository build
checks. The date of the board test, selected Flash Magic file, programming
log, photographs, and measured scores were not supplied. The owner has since
reported that M5 was flashed and physically tested successfully. Two distinct
M5 HEX files exist, so this report does **not** assign either one to the
physical test.

## Configuration recorded

| Item | Evidence and status |
| --- | --- |
| Board | LPC1768 trainer kit; owner-reported M5 physical test. Exact assembly revision not recorded. |
| SW2 | Owner reports a CNB-to-CNB1 FRC cable, P2.12, and active-low behavior. The M5 source enables P2.12 GPIO input with internal pull-up and polls it. Contact continuity, switch voltages, and external pull circuit were not measured. |
| MPU6050 | Existing M4.1 source uses I²C2 on P0.10/P0.11 and address `0x68`; owner reports calibration and gameplay still functional. Physical wire endpoints and I/O levels were not measured. |
| TFT | Existing M4.1 source uses legacy SPI on P0.15/P0.18 with GPIO controls on P0.16/P0.21/P0.22; owner reports gameplay still functional. Exact module electrical compatibility remains unverified. |
| Module supply | Owner previously reported MPU6050 and TFT connected to 5 V. Exact module variants and 5 V input/I/O compatibility have not been established. |
| Score storage | M5 source uses LPC1768 internal flash sectors 14–15 (`0xE000–0xFFFF`) through IAP. Power-off retention was observed by the owner; journal fault handling was not physically tested. |
| Flash Magic | Owner previously reported selecting **Erase Blocks Used by Hex File** only. The exact M5 session settings, selected file, verify result, and programming log are unavailable. Earlier project records list LPC1768, COM5, 9600 baud, 12 MHz oscillator and Verify After Programming for an earlier setup; those are not established as M5 session settings. |

## Owner-reported physical procedures and results

| Procedure described by the owner | Observed result reported by the owner | Detail not supplied |
| --- | --- | --- |
| Run the game through `GAME_OVER`, then use SW2 to restart. | SW2 restart from `GAME_OVER` works without power cycling. | Number of attempts, hold/bounce tests, and screen recording. |
| Play with M5 while using the established M4.1 controls and display. | M4.1 gameplay and calibration remain functional. | Counts of starts, timing, frame-rate measurements, and separate outcomes for each gameplay mechanic. |
| Exercise high-score storage, switch off the board's power supply, then power it on again. | High-score storage works and the saved high score remains after power is restored. | Numeric old/new scores, number of cycles, and whether a lower-scoring game was also tested. |

No failed physical test was reported. This does not mean every acceptance test
passed; tests without an observation above remain unreported.

## Firmware and build identity

The **physically tested HEX is unidentified**: no selected-file path, hash,
Flash Magic log, or board readback was provided. The repository contains:

| Repository artifact | Size | SHA-256 | Evidence status |
| --- | ---: | --- | --- |
| `../../build/2026-10-29/restart_highscore_m5_first/restart_highscore_m5_lpc1768.hex` | 24,320 bytes | `3947A0D4C366D40C262B6ABA388C67B8C7D861DF18CE2F3D367FC0675BCA211A` | First candidate build; no evidence it was flashed. |
| `../../build/2026-10-29/restart_highscore_m5_final/restart_highscore_m5_lpc1768.hex` | 24,460 bytes | `2B6E85CFED81B804C98D59F0245B7F63D59C8437CA0A06AA66373C435D276854` | Final reviewed candidate build; no evidence it was flashed. |

The final build log records ARMCC 5.03 Build 24, successful compile/assembly/
link/FromELF without warnings or errors, 8,876 unique HEX data bytes at
`0x00000000–0x000022AB`, one EOF, and a valid LPC1768 vector checksum. Its
recorded M5 `main.c` SHA-256 is
`F472909DC8E5730325B7367687F78E6EBDECBF3EE1F598EC391E96A0B5787896`,
matching the current source. The build's `memory.sct` reserves sectors 14–15;
the build script rejects HEX data at or above `0xE000`. These are software
checks, not evidence that the final HEX is the board image.

## Unperformed or unresolved checks

- **Firmware reprogramming retention:** no score-sector readback before and
  after a later Flash Magic programming operation was reported. Power-cycle
  retention does not establish that Flash Magic preserves sectors 14–15.
- **SW2 electrical and stress tests:** released/pressed voltages, continuity,
  hold/bounce behavior, presses during gameplay, and repeated restart counts
  were not recorded.
- **Journal fault tests:** interrupted IAP write, power loss during sector
  rollover, invalid-record recovery, and `SAVE FAILED` behavior were not
  physically exercised in the supplied evidence.
- **Source review limitations:** the preceding read-only review identified
  that restart timers are reset before the full-screen redraw, so first-shot
  or spawn timing may be shortened after restart. It also found that the
  journal validates the first 24 bytes of each 256-byte record without a
  separate commit step. No M5 source change or new build was made here.
- **Image identification:** because the flashed HEX is unknown, the observed
  board behavior cannot be tied conclusively to either repository M5 build or
  the current source. A selected-file screenshot, programming log, or board
  readback would close this record gap.

The earlier milestone sources and verified M4.1 HEX were preserved. This
report records the owner's physical results without extending them to tests
or programming properties that were not observed.
