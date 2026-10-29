# Flick & Fly M5 — SW2 restart and persistent best score

The owner now reports a successful M5 flash and physical operation: SW2
restart works and the best score survives switching the board supply off and
on. The exact flashed HEX has not been identified among two repository M5
builds. See [PHYSICAL_VERIFICATION.md](PHYSICAL_VERIFICATION.md) for the
reported procedures, observed results, and remaining evidence gaps.

This isolated image starts from M4.1. Its MPU6050 I²C2 driver, calibration,
tilt settings, TFT transport and dynamic-window drawing, automatic firing,
enemy and collision rules, lives, and score cap are retained. M4.1 and prior
milestone files are not modified by this folder.

## Hardware status and button

The owner **reports** a CNB-to-CNB1 FRC cable and SW2 on LPC1768 P2.12,
pressed low and released high. Continuity, exact released/pressed voltage,
external pull circuit, and unpowered signal level have **not** been measured.
The trainer manuals conflict about the CNB1 contact and switch polarity.
The MPU6050 and TFT have been supplied from 5 V, but their exact modules'
input compatibility and I/O voltage safety are not established. These are
hardware risks, not validated by a firmware build. Review them before any
further programming or wiring changes.

M5 selects P2.12 GPIO (`PINSEL4[25:24]=00`), internal pull-up
(`PINMODE4[25:24]=00`), and input (`FIO2DIR[12]=0`), and disables open-drain
for that pin. It polls the active-low level without an interrupt. A 40 ms
stable press edge is accepted only in `GAME_OVER`; a stable release is
required first, including after startup. A held button yields one event.
Button polling never sleeps or delays a gameplay frame.

## Best-score flash journal

The LPC1768 has no assumed internal EEPROM. M5 uses internal flash sectors
14 (`0xE000–0xEFFF`) and 15 (`0xF000–0xFFFF`), 4 KiB each, as documented by
NXP UM10360, Chapter 32. The alternate onboard U5 SPI EEPROM was not used:
its chip identity, wiring, jumpers and sharing with the TFT are unverified.

Each 256-byte slot is programmed at most once. The first 24 bytes contain
little-endian magic `0x354D4646`, version 1, a monotonically increasing
sequence, score, bitwise score complement, and CRC-32 over the first 20 bytes.
The remainder is `0xFF`. On load, records with wrong format, CRC, sequence,
complement, or score outside 1–99999 are ignored; an erased or corrupt journal
loads best score zero. The highest valid score wins, with sequence as a tie
breaker. A new record is written only at `GAME_OVER` when the final score
exceeds the prior best. No per-frame flash writes occur.

NXP IAP entry `0x1FFF1FF1` receives prepare (50), erase (52), and copy
RAM-to-flash (51) commands. The 256-byte source buffer is word aligned; the
destination is 256-byte aligned; the clock argument is `SystemCoreClock/1000`
kHz. Interrupts are masked for each IAP call and the previous mask restored.
The linker reserves the top 32 bytes of main SRAM for ROM IAP. A write is
accepted only after full 256-byte readback and record validation. When the
active sector fills, the *other* sector is erased and checked blank while the
highest valid record remains in the active sector. The old sector is not
erased after the replacement write; it remains a fallback until a later
rollover. Power loss during a replacement can therefore leave the previous
valid score. If IAP or readback fails, the previous best remains displayed
and `SAVE FAILED` appears; gameplay and SW2 restart remain available.

`memory.sct` limits the entire flash load region to below `0xE000` and the
main RAM region to `0x10000000–0x10007FDF`. The build script also rejects
every Intel HEX data byte at or above `0xE000`, duplicate data addresses,
invalid record checksums/EOF, and an invalid LPC1768 vector checksum.

**Flash Magic preservation is unverified on this board.** The reported
setting is “Erase Blocks Used by Hex File” only. M5's HEX excludes sectors
14–15, but neither that setting nor this build proves the board retains data
through a later firmware programming operation. Before relying on that,
record a best score, read/record the reserved sector contents, program the
same or a newer image with the exact setting, and verify the score/sector
contents afterward. “Erase all” or a different programmer can erase it.

## Restart behavior and physical acceptance

SW2 acts only at `GAME_OVER`. Restart clears score, lives, ship and shot
position, all enemies, lane index, sensor-error and hit indicators, damage
grace, and frame/spawn/shot timers. It redraws the scene and HUD, retains
the successful startup calibration and loaded best score, and resumes
automatic firing. A failed startup calibration still needs a board reset,
as in M4.1.

The owner has reported that SW2 restart works, high-score storage works and
persists across a board power-off/power-on cycle, and M4.1 calibration and
gameplay remain functional. Switch voltages, hold/bounce behavior, lower-score
retention, interrupted writes, repeated-cycle counts, and score preservation
through firmware reprogramming have not been reported as tested. See the
physical verification record for the exact evidence boundary.

## Isolated build

Use a new output directory for every build:

```powershell
C:\Users\Laptop\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe firmware\tests\restart_highscore_m5\build.py --output-dir firmware\build\2026-10-29\restart_highscore_m5_rebuild_1
```

The script checks its vector patch against the preserved combined-test HEX,
then invokes ARMCC 5.03 Build 24, `armasm`, `armlink` with `memory.sct`, and
`fromelf`. Only the final `*_lpc1768.hex` has the LPC1768 vector checksum.

The reviewed final build is
`../../build/2026-10-29/restart_highscore_m5_final/restart_highscore_m5_lpc1768.hex`:
24,460 bytes, SHA-256
`2B6E85CFED81B804C98D59F0245B7F63D59C8437CA0A06AA66373C435D276854`.
It has 8,876 unique data bytes at `0x00000000–0x000022AB`, one EOF, valid
Intel HEX checksums, and a valid LPC1768 vector checksum. The compiler,
assembler, linker, and FromELF reported no warnings or errors. This is a
build result only. The owner has since reported a successful M5 flash and
physical test, but has not identified which M5 HEX was selected.

Sources: NXP [UM10360, Chapter 32](https://www.pololu.com/file/0J431/user.manual.lpc17xx.pdf)
(NXP-authored manual mirrored by Pololu),
[NXP LPC1769/68 product datasheet](https://www.nxp.com/docs/en/data-sheet/LPC1769_68_67_66_65_64_63.pdf),
and the locally supplied ALS trainer manual (PDF pp. 35–37).
