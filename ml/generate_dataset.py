"""Deterministic generated examples for the M6 offline demonstration.

Values are illustrative calibrated X-axis counts, not MPU6050 recordings.
The label rule follows the M5 plus/minus 150-count neutral dead zone.
"""

import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parent
DATASET = ROOT / "data" / "synthetic_tilt.csv"
DEAD_ZONE = 150
MIN_DELTA = -600
MAX_DELTA = 600
LABELS = ("LEFT_TILT", "NEUTRAL", "RIGHT_TILT")
BOUNDARIES = (-151, -150, 150, 151)
NEAR_BOUNDARY_TEST = (-152, -149, 149, 152)


def label_for_delta(delta_x):
    if delta_x < -DEAD_ZONE:
        return "LEFT_TILT"
    if delta_x > DEAD_ZONE:
        return "RIGHT_TILT"
    return "NEUTRAL"


def split_for_delta(delta_x):
    # Boundary values stay in training; hold out a fixed subset elsewhere.
    if delta_x in BOUNDARIES:
        return "train"
    if delta_x in NEAR_BOUNDARY_TEST or delta_x % 7 == 0:
        return "test"
    return "train"


def generate_rows():
    return [(delta_x, label_for_delta(delta_x), split_for_delta(delta_x))
            for delta_x in range(MIN_DELTA, MAX_DELTA + 1)]


def write_dataset(path=DATASET):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(("delta_x", "label", "split"))
        writer.writerows(generate_rows())
    return path


if __name__ == "__main__":
    path = write_dataset()
    print(f"generated {len(generate_rows())} synthetic rows: {path}")
