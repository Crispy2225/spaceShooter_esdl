"""Generated-data, model, baseline, and compiled-C host checks."""

import csv
import ctypes
import io
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from generate_dataset import (BOUNDARIES, DATASET, LABELS, generate_rows,
                              label_for_delta)
from train_export import REPORT, MODEL_C, evaluate, fit_model, load_dataset, render_c
from update_docs import START, END, render_html, render_markdown


class TiltTreeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = shutil.which("gcc")
        if compiler is None:
            raise RuntimeError("gcc is required for generated-C host tests")
        cls.rows = load_dataset()
        cls.model = fit_model(cls.rows)
        cls.temp = tempfile.TemporaryDirectory()
        library = Path(cls.temp.name) / "tilt_tree.dll"
        subprocess.run([compiler, "-std=c89", "-Wall", "-Wextra", "-Werror",
                        "-shared", "-o", str(library), str(MODEL_C)], check=True)
        cls.library = ctypes.CDLL(str(library))
        cls.predict_c = cls.library.tilt_tree_predict
        cls.predict_c.argtypes = [ctypes.c_long]
        cls.predict_c.restype = ctypes.c_int

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "temp"):
            if hasattr(ctypes, "windll"):
                import _ctypes
                _ctypes.FreeLibrary(cls.library._handle)
            del cls.predict_c
            del cls.library
            cls.temp.cleanup()

    def test_dataset_is_reproducible_and_disjoint(self):
        buffer = io.StringIO(newline="")
        writer = csv.writer(buffer, lineterminator="\n")
        writer.writerow(("delta_x", "label", "split"))
        writer.writerows(generate_rows())
        self.assertEqual(DATASET.read_bytes(), buffer.getvalue().encode("utf-8"))
        self.assertEqual(len(self.rows), 1201)
        self.assertEqual(len({delta for delta, _, _ in self.rows}), 1201)
        self.assertEqual({split for _, _, split in self.rows}, {"train", "test"})
        self.assertEqual({label for _, label, _ in self.rows}, set(LABELS))
        for delta in BOUNDARIES:
            self.assertEqual(next(split for value, _, split in self.rows if value == delta), "train")

    def test_class_examples_and_boundaries(self):
        cases = (-32768, -600, -152, -151, -150, -149, -1, 0, 1,
                 149, 150, 151, 152, 600, 32767)
        for delta in cases:
            with self.subTest(delta=delta):
                target = label_for_delta(delta)
                self.assertEqual(self.model.predict([[delta]])[0], target)
                self.assertEqual(LABELS[self.predict_c(delta)], target)

    def test_generated_holdout_tree_baseline_and_c(self):
        held_out = [(delta, label) for delta, label, split in self.rows if split == "test"]
        self.assertGreater(len(held_out), 100)
        for delta, actual in held_out:
            with self.subTest(delta=delta):
                self.assertEqual(self.model.predict([[delta]])[0], actual)
                self.assertEqual(label_for_delta(delta), actual)
                self.assertEqual(LABELS[self.predict_c(delta)], actual)

    def test_export_and_report_reproduce(self):
        self.assertEqual(MODEL_C.read_text(encoding="utf-8"), render_c(self.model))
        self.assertEqual(json.loads(REPORT.read_text(encoding="utf-8")),
                         evaluate(self.model, self.rows))
        self.assertEqual(self.model.get_depth(), 2)
        self.assertEqual(self.model.get_n_leaves(), 3)

    def test_documentation_results_match_report(self):
        report = json.loads(REPORT.read_text(encoding="utf-8"))
        markdown = (ROOT / "README.md").read_text(encoding="utf-8")
        website = (ROOT.parent / "index.html").read_text(encoding="utf-8")
        markdown_block = markdown.split(START, 1)[1].split(END, 1)[0].strip()
        html_block = website.split(START, 1)[1].split(END, 1)[0].strip()
        self.assertEqual(markdown_block, render_markdown(report).strip())
        self.assertEqual("\n".join(line.strip() for line in html_block.splitlines()),
                         "\n".join(line.strip() for line in render_html(report).strip().splitlines()))


if __name__ == "__main__":
    unittest.main()
