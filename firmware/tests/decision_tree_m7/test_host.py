"""Host-check the exact M5 and M7 steering functions without board hardware."""

import ctypes
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest


HERE = Path(__file__).resolve().parent
M5 = HERE.parent / "restart_highscore_m5" / "main.c"
M7 = HERE / "main.c"


def steering_source(path):
    source = path.read_text(encoding="utf-8")
    constants = []
    for name in ("DEAD_ZONE", "COUNTS_PER_STEP", "SHIP_LEFT", "SHIP_RIGHT"):
        match = re.search(r"^#define\s+" + name + r"\s+(\d+[UL]*)", source, re.M)
        if match is None:
            raise AssertionError(f"Missing {name} in {path}")
        constants.append(f"#define {name} {match.group(1)}")
    match = re.search(
        r"static unsigned int next_ship_x\(unsigned int current, long delta\)\s*\{.*?\n\}",
        source, re.S)
    if match is None:
        raise AssertionError(f"Missing steering function in {path}")
    function = match.group(0).replace("static unsigned int", "unsigned int", 1)
    includes = '#include "tilt_tree.h"\n' if path == M7 else ""
    return includes + "\n".join(constants) + "\n" + function + "\n"


class SteeringParityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = shutil.which("gcc")
        if compiler is None:
            raise RuntimeError("Host GCC is required for M7 parity tests")
        cls.temp = tempfile.TemporaryDirectory()
        cls.libraries = []
        cls.functions = []
        for name, path, extra in (("m5", M5, []),
                                  ("m7", M7, [str(HERE / "tilt_tree.c")])):
            source_file = Path(cls.temp.name) / f"{name}_steering.c"
            source_file.write_text(steering_source(path), encoding="utf-8")
            library_file = Path(cls.temp.name) / f"{name}_steering.dll"
            subprocess.run([compiler, "-std=c89", "-pedantic-errors", "-Wall",
                            "-Wextra", "-Werror", "-I", str(HERE), "-shared",
                            "-o", str(library_file), str(source_file), *extra], check=True)
            library = ctypes.CDLL(str(library_file))
            cls.libraries.append(library)
            function = library.next_ship_x
            function.argtypes = [ctypes.c_uint, ctypes.c_long]
            function.restype = ctypes.c_uint
            cls.functions.append(function)
        cls.tree = cls.libraries[1].tilt_tree_predict
        cls.tree.argtypes = [ctypes.c_long]
        cls.tree.restype = ctypes.c_int

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "temp"):
            if hasattr(ctypes, "windll"):
                import _ctypes
                for library in cls.libraries:
                    _ctypes.FreeLibrary(library._handle)
            del cls.tree
            cls.functions.clear()
            cls.libraries.clear()
            cls.temp.cleanup()

    def test_tree_boundaries(self):
        expected = {-65535: 0, -151: 0, -150: 1, 0: 1,
                    150: 1, 151: 2, 65535: 2}
        for delta, label in expected.items():
            with self.subTest(delta=delta):
                self.assertEqual(self.tree(delta), label)

    def test_example_movement_and_clamps(self):
        expected = [((55, -150), 55), ((55, -151), 54),
                    ((55, -1650), 53), ((55, 151), 56),
                    ((55, 1650), 57), ((55, 6150), 60),
                    ((8, -6000), 8), ((103, 6000), 103)]
        for arguments, position in expected:
            with self.subTest(arguments=arguments):
                self.assertEqual(self.functions[0](*arguments), position)
                self.assertEqual(self.functions[1](*arguments), position)

    def test_all_possible_raw_minus_neutral_deltas_at_representative_positions(self):
        # Signed 16-bit raw and signed 16-bit neutral imply this delta range.
        for position in (8, 9, 55, 102, 103):
            for delta in range(-65535, 65536):
                if self.functions[0](position, delta) != self.functions[1](position, delta):
                    self.fail(f"M5/M7 differ at position={position}, delta={delta}")


if __name__ == "__main__":
    unittest.main()
