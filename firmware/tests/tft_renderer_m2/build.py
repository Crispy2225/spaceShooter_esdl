r"""Isolated M2 ARMCC 5.03 build using the recovered 2026-10-27 commands.

Requires the installed C:\Keil\ARM toolchain. The checksum routine is checked
against the preserved combined-test raw/final HEX pair before building M2.
No existing output directory is ever reused.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import subprocess
import sys


HERE = Path(__file__).resolve().parent
FIRMWARE = HERE.parent.parent
BUILD_ROOT = FIRMWARE / "build"
KEIL = Path(r"C:\Keil\ARM")
BIN = KEIL / "ARMCC" / "bin"
SUPPORT = KEIL / "Startup" / "NXP" / "LPC17xx"
INCLUDES = [KEIL / "INC" / "NXP" / "LPC17xx", KEIL / "CMSIS" / "Include"]
BASELINE = BUILD_ROOT / "2026-10-27" / "mpu6050_tft_test"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def patch_vector(raw: bytes) -> tuple[bytes, int, int]:
    """Patch LPC1768 vector word 7 and all affected Intel HEX checksums."""
    lines = raw.decode("ascii").splitlines()
    records: list[tuple[int, int, bytearray]] = []
    vector: dict[int, int] = {}
    upper = 0
    eof_count = 0
    for line in lines:
        if not line.startswith(":"):
            raise ValueError("Non-HEX line")
        record = bytes.fromhex(line[1:])
        if len(record) < 5 or len(record) != record[0] + 5 or sum(record) & 0xFF:
            raise ValueError("Invalid Intel HEX record")
        address = (record[1] << 8) | record[2]
        kind = record[3]
        data = bytearray(record[4:-1])
        if kind == 0x04:
            if len(data) != 2:
                raise ValueError("Invalid extended linear address")
            upper = ((data[0] << 8) | data[1]) << 16
        elif kind == 0x00:
            for offset, value in enumerate(data):
                absolute = upper + address + offset
                if 0 <= absolute < 32:
                    if absolute in vector:
                        raise ValueError("Overlapping vector bytes")
                    vector[absolute] = value
        elif kind == 0x01:
            eof_count += 1
        elif kind == 0x05:
            if len(data) != 4:
                raise ValueError("Invalid start linear address")
        else:
            raise ValueError(f"Unexpected Intel HEX record type {kind}")
        records.append((address, kind, data))
    if eof_count != 1 or set(vector) != set(range(32)):
        raise ValueError("Missing EOF or complete vector table")

    def word(start: int) -> int:
        return int.from_bytes(bytes(vector[start + i] for i in range(4)), "little")

    if word(28) != 0:
        raise ValueError("Raw vector checksum word is not zero")
    checksum = (-sum(word(i) for i in range(0, 28, 4))) & 0xFFFFFFFF
    patch = checksum.to_bytes(4, "little")
    output = []
    upper = 0
    patched_bytes = 0
    for address, kind, data in records:
        if kind == 0x04:
            upper = ((data[0] << 8) | data[1]) << 16
        if kind == 0x00:
            for offset in range(len(data)):
                absolute = upper + address + offset
                if 28 <= absolute < 32:
                    data[offset] = patch[absolute - 28]
                    patched_bytes += 1
        head = bytearray([len(data), address >> 8, address & 0xFF, kind]) + data
        output.append(":" + head.hex().upper() + f"{(-sum(head)) & 0xFF:02X}")
    if patched_bytes != 4 or (sum(word(i) for i in range(0, 28, 4)) + checksum) & 0xFFFFFFFF:
        raise ValueError("Vector checksum validation failed")
    return ("\n".join(output) + "\n").encode("ascii"), checksum, len(output)


def run_tool(log, cwd: Path, executable: Path, args: list[str]) -> None:
    command = [str(executable), *args]
    log.write("COMMAND: " + subprocess.list2cmdline(command) + "\n")
    result = subprocess.run(command, cwd=cwd, capture_output=True, text=True, errors="replace")
    log.write(result.stdout)
    log.write(result.stderr)
    log.write(f"EXIT: {result.returncode}\n")
    log.flush()
    if result.returncode:
        raise RuntimeError(f"{executable.name} failed with exit {result.returncode}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=BUILD_ROOT / "2026-10-29" / "tft_renderer_m2")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    try:
        output.relative_to(BUILD_ROOT.resolve())
    except ValueError as exc:
        raise SystemExit("Output directory must be under firmware/build") from exc
    if output.exists():
        raise SystemExit(f"Refusing to overwrite existing directory: {output}")

    baseline_raw = BASELINE / "mpu6050_tft_test_raw.hex"
    baseline_final = BASELINE / "mpu6050_tft_test_lpc1768.hex"
    rebuilt_baseline, baseline_checksum, baseline_records = patch_vector(baseline_raw.read_bytes())
    if rebuilt_baseline != baseline_final.read_bytes() or baseline_checksum != 0xEFFFF44E:
        raise SystemExit("Checksum procedure does not reproduce the preserved combined HEX")

    source = HERE / "main.c"
    system = SUPPORT / "system_LPC17xx.c"
    startup = SUPPORT / "startup_LPC17xx.s"
    header = INCLUDES[0] / "LPC17xx.h"
    for required in [source, system, startup, header, *INCLUDES]:
        if not required.exists():
            raise SystemExit(f"Missing build input: {required}")

    output.mkdir(parents=True)
    with (output / "build.log").open("x", encoding="utf-8") as log:
        log.write("Evidence: board_smoke_single, lcd_test and mpu6050_lcd_test logs; isolated combined raw HEX reproduced byte-for-byte.\n")
        log.write(f"Baseline checksum matched: 0x{baseline_checksum:08X}; {baseline_records} records\n")
        for name, path in [("M2 source", source), ("system", system), ("startup", startup), ("LPC17xx.h", header)]:
            log.write(f"{name}: {path}; SHA-256 {sha256(path)}\n")
        for tool in ["armcc.exe", "armasm.exe", "armlink.exe", "fromelf.exe"]:
            run_tool(log, output, BIN / tool, ["--vsn"])
        cc = ["--cpu=Cortex-M3", "-O0", "--c99", "--apcs=interwork",
              *["-I" + str(path) for path in INCLUDES], "-c"]
        run_tool(log, output, BIN / "armcc.exe", cc + [str(source), "-o", "main.o"])
        run_tool(log, output, BIN / "armcc.exe", cc + [str(system), "-o", "system_LPC17xx.o"])
        run_tool(log, output, BIN / "armasm.exe", ["--cpu=Cortex-M3", str(startup), "-o", "startup_LPC17xx.o"])
        run_tool(log, output, BIN / "armlink.exe", [
            "--cpu=Cortex-M3", "--ro-base=0x00000000", "--rw-base=0x10000000",
            "--entry=Reset_Handler", "--no_remove", "--first=startup_LPC17xx.o(RESET)",
            "startup_LPC17xx.o", "system_LPC17xx.o", "main.o", "-o", "tft_renderer_m2.axf"])
        run_tool(log, output, BIN / "fromelf.exe", ["--i32combined", "--output",
            "tft_renderer_m2_raw.hex", "tft_renderer_m2.axf"])
        final_bytes, checksum, records = patch_vector((output / "tft_renderer_m2_raw.hex").read_bytes())
        final = output / "tft_renderer_m2_lpc1768.hex"
        with final.open("xb") as stream:
            stream.write(final_bytes)
        log.write(f"VECTOR CHECKSUM: 0x{checksum:08X}; {records} valid HEX records\n")
        log.write(f"FINAL HEX: {final}; {final.stat().st_size} bytes; SHA-256 {sha256(final)}\n")
        log.flush()
    print(f"Build complete: {final}")
    print(f"SHA-256: {sha256(final)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
