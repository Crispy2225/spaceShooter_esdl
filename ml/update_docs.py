"""Refresh bounded M6 result blocks in the README and player guide."""

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parent
REPORT = ROOT / "evaluation.json"
MARKDOWN = ROOT / "README.md"
WEBSITE = ROOT.parent / "index.html"
START = "<!-- M6_RESULTS_START -->"
END = "<!-- M6_RESULTS_END -->"


def result_values(report):
    rows = report["rows"]
    tree = report["tree_on_generated_holdout"]
    baseline = report["baseline_on_generated_holdout"]
    thresholds = sorted(report["model"]["split_thresholds"])
    if len(thresholds) != 2 or rows["test"] <= 0:
        raise ValueError("Expected a two-split tree and nonempty holdout")
    left_cut, neutral_cut = (int(value // 1) for value in thresholds)
    return rows, tree, baseline, left_cut, neutral_cut


def render_markdown(report):
    rows, tree, baseline, left_cut, neutral_cut = result_values(report)
    matrix = tree["confusion_matrix"]
    samples = "\n".join(
        f"| `{sample['delta_x']}` | `{sample['tree']}` | `{sample['baseline']}` |"
        for sample in report["samples"]
    )
    return (
        f"The generated dataset has **{rows['total']:,} rows**: **{rows['train']:,} training** and "
        f"**{rows['test']:,} fixed held-out generated examples**. The depth-{report['model']['depth']} tree "
        f"has {report['model']['leaves']} leaves and split points "
        f"`{min(report['model']['split_thresholds']):g}` and "
        f"`{max(report['model']['split_thresholds']):g}` raw counts. Its integer C rules are "
        f"`delta_x <= {left_cut}` → `LEFT_TILT`, "
        f"`{left_cut + 1}..{neutral_cut}` → `NEUTRAL`, and "
        f"`delta_x >= {neutral_cut + 1}` → `RIGHT_TILT`.\n\n"
        "| Generated holdout result | Decision tree | Threshold baseline |\n"
        "| --- | ---: | ---: |\n"
        f"| Accuracy | {tree['accuracy']:.3f} | {baseline['accuracy']:.3f} |\n"
        f"| Macro F1 | {tree['macro_f1']:.3f} | {baseline['macro_f1']:.3f} |\n"
        f"| Prediction agreement | {report['tree_baseline_agreement']:.3f} | — |\n\n"
        f"Tree holdout confusion matrix in LEFT/NEUTRAL/RIGHT order: "
        f"`{matrix}`. The baseline matrix is the same.\n\n"
        "| Calibrated X delta | Tree | Baseline |\n"
        "| ---: | --- | --- |\n"
        f"{samples}\n\n"
        "**Interpretation:** The labels were generated from the threshold baseline rule. "
        "Both methods being equivalent on this generated set is expected and does not "
        "establish superiority or real-sensor accuracy.\n"
    )


def render_html(report):
    rows, tree, baseline, left_cut, neutral_cut = result_values(report)
    samples = ", ".join(
        f"{sample['delta_x']} → {sample['tree']}" for sample in report["samples"]
    )
    return (
        "<div class=\"ml-results\" aria-label=\"Generated M6 evaluation\">\n"
        "  <h3>What the generated test shows</h3>\n"
        f"  <p>{rows['total']:,} generated values: {rows['train']:,} for training and "
        f"{rows['test']:,} held out. The depth-{report['model']['depth']} tree has "
        f"{report['model']['leaves']} leaves.</p>\n"
        "  <div class=\"ml-rule-grid\" aria-label=\"Learned integer rules\">"
        f"<div><code>≤ {left_cut}</code><strong>LEFT_TILT</strong></div>"
        f"<div><code>{left_cut + 1} to {neutral_cut}</code><strong>NEUTRAL</strong></div>"
        f"<div><code>≥ {neutral_cut + 1}</code><strong>RIGHT_TILT</strong></div></div>\n"
        f"  <p>Generated holdout: tree accuracy {tree['accuracy']:.3f} and macro F1 "
        f"{tree['macro_f1']:.3f}; threshold baseline accuracy {baseline['accuracy']:.3f} "
        f"and macro F1 {baseline['macro_f1']:.3f}. Prediction agreement: "
        f"{report['tree_baseline_agreement']:.3f}. Examples: {samples}.</p>\n"
        "  <p>The labels come from that threshold rule, so the matching results are "
        "expected. These values do not measure real sensor accuracy.</p>\n"
        "</div>\n"
    )


def replace_block(path, replacement):
    original = path.read_bytes().decode("utf-8")
    if original.count(START) != 1 or original.count(END) != 1:
        raise ValueError(f"Expected one bounded M6 result block in {path}")
    before, remainder = original.split(START, 1)
    _, after = remainder.split(END, 1)
    newline = "\r\n" if "\r\n" in original else "\n"
    indent = before.rsplit("\n", 1)[-1].replace("\r", "")
    updated = before + START + newline + replacement.replace("\n", newline) + indent + END + after
    path.write_bytes(updated.encode("utf-8"))


def main():
    report = json.loads(REPORT.read_text(encoding="utf-8"))
    replace_block(MARKDOWN, render_markdown(report))
    html = "".join("        " + line for line in render_html(report).splitlines(keepends=True))
    replace_block(WEBSITE, html)
    print(f"updated M6 results in {MARKDOWN} and {WEBSITE}")


if __name__ == "__main__":
    main()
