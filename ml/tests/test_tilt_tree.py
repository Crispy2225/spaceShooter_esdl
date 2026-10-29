"""Host tests for the checked-in C tree and optional sklearn export parity."""

import ctypes
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


def expected(delta):
    return 0 if delta <= -150 else 2 if delta >= 150 else 1


class TiltTreeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = shutil.which("gcc")
        if compiler is None:
            raise unittest.SkipTest("gcc unavailable for C host tests")
        cls.temp = tempfile.TemporaryDirectory()
        library = Path(cls.temp.name) / "tilt_tree.dll"
        subprocess.run([compiler, "-std=c89", "-Wall", "-Wextra", "-Werror",
                        "-shared", "-o", str(library), str(ROOT / "tilt_tree.c")],
                       check=True)
        cls.library = ctypes.CDLL(str(library))
        cls.predict = cls.library.tilt_tree_predict
        cls.predict.argtypes = [ctypes.c_long]
        cls.predict.restype = ctypes.c_int

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "temp"):
            # Windows keeps a DLL locked while ctypes holds the library handle.
            if hasattr(ctypes, "windll"):
                import _ctypes
                _ctypes.FreeLibrary(cls.library._handle)
            del cls.predict
            del cls.library
            cls.temp.cleanup()

    def test_all_classes_and_boundaries(self):
        for delta in (-32768, -600, -151, -150, -149, -1, 0, 1,
                      148, 149, 150, 151, 600, 32767):
            with self.subTest(delta=delta):
                self.assertEqual(self.predict(delta), expected(delta))

    def test_full_synthetic_domain(self):
        for delta in range(-600, 601):
            self.assertEqual(self.predict(delta), expected(delta))

    def test_sklearn_export_parity_when_available(self):
        try:
            from sklearn.tree import DecisionTreeClassifier
        except ImportError:
            self.skipTest("scikit-learn unavailable; train/export parity not run")
        import sys
        sys.path.insert(0, str(ROOT))
        from train_export import render_c, synthetic_rows
        features, labels = synthetic_rows()
        model = DecisionTreeClassifier(max_depth=2, random_state=0).fit(features, labels)
        self.assertEqual((ROOT / "tilt_tree.c").read_text(encoding="utf-8"), render_c(model))
        for delta in range(-600, 601):
            name = model.predict([[delta]])[0]
            self.assertEqual(self.predict(delta), ("LEFT_TILT", "NEUTRAL", "RIGHT_TILT").index(name))


if __name__ == "__main__":
    unittest.main()
