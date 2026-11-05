"""Compile and run the Language value and C adapter owner tests."""

from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]
LANGUAGE = ROOT / "suite/ecosystem/language"
VEXSPOKE = ROOT / "suite/ecosystem/vexspoke/src"
ADAPTER_RUNNER = Path(__file__).resolve().parent / "adapters/c/c23_lexer_run.py"
CASES = (
    ("document/source_range", ("document/source_range.c",)),
    ("token/token", ("token/token.c", "document/source_range.c")),
    ("semantic/symbol", ("semantic/symbol.c", "document/source_range.c")),
    ("semantic/reference", ("semantic/reference.c", "document/source_range.c")),
    ("adapters/diff/diff_adapter", (
        "adapters/diff/diff_adapter.c", "token/token.c", "document/source_range.c"
    )),
    ("text/bounded_text", ("text/bounded_text.c",)),
    ("style/text_style", ("style/text_style.c", "text/bounded_text.c")),
    ("style/style_key_registry", ("style/style_key_registry.c", "text/bounded_text.c")),
    ("syntax/role", ("syntax/role.c",)),
    ("analysis/diagnostic", ("analysis/diagnostic.c", "text/bounded_text.c", "document/source_range.c")),
    ("analysis/syntax_span", ("analysis/syntax_span.c", "text/bounded_text.c", "document/source_range.c", "syntax/role.c")),
)


def main():
    with tempfile.TemporaryDirectory(prefix="language-test-") as directory:
        output = Path(directory)
        configurations = (
            ("strict", ()),
            ("asan-ubsan", ("-fsanitize=address,undefined", "-fno-omit-frame-pointer")),
        )
        for configuration, flags in configurations:
            for unit, sources in CASES:
                test = ROOT / "language/test" / f"{unit}_test.c"
                binary = output / f"{configuration}_{unit.replace('/', '_')}"
                command = [
                    "clang", "-std=c23", "-Wall", "-Wextra", "-Werror", *flags,
                    f"-I{LANGUAGE}", f"-I{VEXSPOKE}", str(test),
                    *(str(LANGUAGE / source) for source in sources), "-o", str(binary),
                ]
                subprocess.run(command, check=True)
                result = subprocess.run([str(binary)], check=True, capture_output=True, text=True)
                expected_reports = 9 if unit == "adapters/diff/diff_adapter" else 0
                reports = result.stderr.count("[vex] diff_adapter.c:")
                if reports != expected_reports:
                    raise AssertionError(
                        f"{unit}/{configuration}: expected {expected_reports} rejection diagnostics, "
                        f"got {reports}: {result.stderr!r}"
                    )
                if expected_reports and (
                    "unrecognized unified diff line" not in result.stderr
                    or "invalid line span" not in result.stderr
                    or "empty diff line" not in result.stderr
                ):
                    raise AssertionError(
                        f"{unit}/{configuration}: expected rejection messages missing: {result.stderr!r}"
                    )
                if result.stdout:
                    print(result.stdout, end="")
        subprocess.run([sys.executable, str(ADAPTER_RUNNER)], check=True)
        subprocess.run([
            sys.executable,
            str(Path(__file__).resolve().parent / "adapters/c23/c23_adapter_run.py"),
        ], check=True)
        print(f"PASS: {len(CASES)} Language value owners + C23 adapter owners (strict + ASan/UBSan)")


if __name__ == "__main__":
    main()
