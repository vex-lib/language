"""Strict and sanitizer owners for SyntaxRole, C23LexStep, C23Lexer and hints."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
WORKSPACE = Path(os.environ.get("VEX_WORKSPACE_ROOT", ROOT.parent))
LANGUAGE = Path(os.environ.get("C23_LANGUAGE_ROOT", WORKSPACE / "suite/ecosystem/language"))
SHARED_LANGUAGE = Path(os.environ.get("C23_SHARED_LANGUAGE_ROOT", LANGUAGE))
VEXSPOKE = WORKSPACE / "suite/ecosystem/vexspoke/src"
TESTS = ROOT / "language"

CASES = (
    ("syntax_role", "syntax/role_test.c", ("syntax/role.c",)),
    ("c23_lex_step", "adapters/c/c23_lex_step_test.c", (
        "adapters/c/c23_lex_step.c", "syntax/role.c", "analysis/syntax_span.c",
        "document/source_range.c", "text/bounded_text.c",
    )),
    ("c23_lexer", "adapters/c/c23_lexer_test.c", (
        "adapters/c/c23_lexer.c", "adapters/c/c23_lex_step.c",
        "syntax/role.c", "analysis/syntax_span.c", "document/source_range.c",
        "text/bounded_text.c",
    )),
    ("c23_inlay_hints", "adapters/c/c23_inlay_hints_test.c", (
        "adapters/c/c23_inlay_hints.c", "adapters/c/c23_lexer.c",
        "adapters/c/c23_lex_step.c", "syntax/role.c", "analysis/syntax_span.c",
        "document/source_range.c", "text/bounded_text.c",
    )),
    ("c23_highlights", "adapters/c/c23_highlights_test.c", (
        "adapters/c/c23_highlights.c", "adapters/c/c23_lexer.c",
        "adapters/c/c23_lex_step.c", "syntax/role.c", "analysis/syntax_span.c",
        "document/source_range.c", "text/bounded_text.c",
    )),
)

EXPECTATIONS = {
    "c23_lexer": (6, "c23_lexer.c", ("rejected source admission",
                                     "rejected unsupported lexical profile")),
    "c23_inlay_hints": (1, "c23_inlay_hints.c", ("rejected source admission",)),
    "c23_highlights": (2, "c23_highlights.c", ("rejected source admission",)),
}

with tempfile.TemporaryDirectory(prefix="c23-lexer-test-") as temporary:
    for mode, flags in (("strict", ()), ("asan-ubsan", ("-fsanitize=address,undefined", "-fno-omit-frame-pointer"))):
        for name, test_name, sources in CASES:
            binary = Path(temporary) / f"{mode}-{name}"
            command = [
                "clang", "-std=c23", "-Wall", "-Wextra", "-Werror", *flags,
                f"-I{LANGUAGE}", f"-I{SHARED_LANGUAGE}", f"-I{VEXSPOKE}",
                str(TESTS / test_name),
                *(str(LANGUAGE / source) if (LANGUAGE / source).exists()
                  else str(SHARED_LANGUAGE / source) for source in sources),
                "-o", str(binary),
            ]
            result = subprocess.run(command, capture_output=True, text=True)
            if result.returncode != 0:
                raise RuntimeError(f"compile failed: {' '.join(command)}\n{result.stderr}")
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            if result.returncode != 0:
                raise RuntimeError(f"{mode} {name} failed: {result.stderr}")
            expected, diagnostic_source, expected_markers = EXPECTATIONS.get(
                name, (0, "c23_lexer.c", ()))
            actual = result.stderr.count(f"[vex] {diagnostic_source}:")
            if actual != expected:
                raise AssertionError(f"{mode} {name}: expected {expected} lexer diagnostics, got {actual}: {result.stderr!r}")
            if expected and any(marker not in result.stderr for marker in expected_markers):
                raise AssertionError(f"{mode} {name}: expected diagnostic messages missing: {result.stderr!r}")
            print(f"PASS {mode} {name}")

    for label, expression in (
        ("lexer-one-argument", "C23Lexer(1u)"),
        ("lexer-two-argument", 'C23Lexer(1u, "x")'),
        ("lexer-four-argument", 'C23Lexer(1u, "x", 1u, 4u)'),
        ("lex-step-one-argument", "C23LexStep(true)"),
        ("lex-step-three-argument", "C23LexStep(true, (SyntaxSpan) {0}, 1u)"),
        ("inlay-hints-one-argument", "C23InlayHints(1u)"),
        ("inlay-hints-two-argument", 'C23InlayHints(1u, "x")'),
        ("inlay-hints-four-argument", 'C23InlayHints(1u, "x", 1u, 4u)'),
        ("highlights-one-argument", "C23Highlights(1u)"),
        ("highlights-two-argument", 'C23Highlights(1u, "x")'),
        ("highlights-four-argument", 'C23Highlights(1u, "x", 1u, 4u)'),
    ):
        source = Path(temporary) / f"{label}.c"
        source.write_text(
            '#include "adapters/c/c23_lexer.h"\n'
            '#include "adapters/c/c23_inlay_hints.h"\n'
            '#include "adapters/c/c23_highlights.h"\n'
            f'int main(void) {{ (void) {expression}; return 0; }}\n',
            encoding="utf-8",
        )
        result = subprocess.run([
            "clang", "-std=c23", "-Wall", "-Wextra", "-Werror", "-fsyntax-only",
            f"-I{LANGUAGE}", f"-I{SHARED_LANGUAGE}", f"-I{VEXSPOKE}", str(source),
        ], capture_output=True, text=True)
        if result.returncode == 0 or "invalid_arity" not in result.stderr:
            raise AssertionError(f"{label}: expected constructor arity rejection, got {result.stderr!r}")
        print(f"PASS compile-negative {label}")
