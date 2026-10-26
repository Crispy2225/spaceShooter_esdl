# 27 October 2026: pre-integration lab gates

Record operator, date/time, board and module IDs, source/build identifier,
photos/log names and the actual result at each gate. A checkbox alone is not
evidence. `manual/hardware-readiness.md` remains the electrical review record.

| Gate | Prerequisites and program | Expected observation; evidence to save | Failure meaning; continue? |
| --- | --- | --- | --- |
| 1. Identify board | Current trainer and chip in hand; compare Lab 6/Appendix C and the hardware-readiness report. No program. | Readable chip/base/piggyback/revision photos and a matching manual or schematic. | Mismatch means project and procedure may target the wrong assembly. Stop board-specific work. |
| 2. Identify programming path | Gate 1. Inspect actual lab PC, programmer, cable and trainer programming controls; compare Lab 6. | Tool/version, cable route, COM/ISP settings and operator recorded. | Missing or inconsistent path blocks flashing. Source preparation may continue; no board pass. |
| 3. Build and program board-only | Gates 1-2; review actual board supply, fitted oscillator, startup/system configuration and programming route before applying power; use `tests/board_smoke/main.c` and reviewed Keil project/HEX output. | Successful build and programmer verification tied to the exact HEX. Save supply/clock review, log and firmware ID. | Uncertain supply or clock blocks power/programming; build error is a software/configuration issue and programming error is an access/procedure issue. Do not start peripheral tests. |
| 4. Observe board LEDs | Gate 3; reviewed CNA-to-CNA1 cable orientation and trainer supply. | Repeatable sequential LED-on then LED-off cycle after reset; save video/photo and trial count. | Blank/static LEDs may be code, routing, power or board. Do not claim board pass or proceed to modules. |
| 5. Approve connections electrically | Gates 1-4; exact MPU/TFT circuit evidence, current jumpers/ribbons, connector-1 and adapter continuity, reviewed voltage/current/pull-up/backlight plan. No program. | Signed module-pin-to-contact and supply review, unpowered continuity/short checks, separate explicit approval for physical wiring. | Missing evidence means no module connection or power. Software correctness does not substitute for this gate. |
| 6. MPU alone | Gate 5 **for MPU**, a reviewed real replacement for `platform_unconfigured.c`, proven diagnostic output. | WHO_AM_I, selected address, repeated timestamped six-axis raw signed values, success/error counts and full trace. | NACK, timeout, wrong ID or unstable reads are failures, not zero measurements. Stop MPU and combined tests; investigate one layer at a time. |
| 7. TFT alone | Gate 5 **for TFT**, identified controller, documented driver and reviewed real replacement for `tft_port_unconfigured.c`. | Expected color/text/four-corner pattern repeatedly visible; save photos, controller evidence and build ID. | Blank/wrong pattern may be backlight, controller, routing, timing or code. Stop TFT and combined tests. |
| 8. Combine | Independent gates 6-7 passed and combined pin/bus/power allocation reviewed. No combined firmware is supplied tonight. | Repeat both standalone observations under combined operation, logs and video. | Any regression or electrical symptom returns work to standalone diagnosis; stop integration. |

**Stop rule:** Documented connector mappings are candidate signals, not
permission to connect or power a module. A successful build is a software
result; an electrically approved connection and a physical observation are
separate results.
