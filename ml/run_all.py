"""One command for generated data, training, evaluation, export, and tests."""

import subprocess
import sys
from pathlib import Path

from train_export import main
from update_docs import main as update_docs


if __name__ == "__main__":
    main()
    update_docs()
    root = Path(__file__).resolve().parents[1]
    subprocess.run([sys.executable, "-m", "unittest", "discover", "-s",
                    "ml/tests", "-v"], cwd=root, check=True)
