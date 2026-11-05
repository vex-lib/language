"""Strict and sanitizer integration proof for the explicit C23 engine adapter."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[4]
LANGUAGE = ROOT / "suite/ecosystem/language"
VEXSPOKE = ROOT / "suite/ecosystem/vexspoke/src"
BASE_SOURCES = (
    "adapters/c23/c23_adapter.c", "adapters/c/c23_lexer.c",
    "adapters/c/c23_lex_step.c", "lang/language_adapter.c",
    "lang/language_lex_result.c", "lang/language_registry.c",
    "lang/language_engine.c", "document/text_snapshot.c", "analysis/syntax_span.c",
    "document/source_range.c", "syntax/role.c", "text/bounded_text.c",
)
CASES = (
    ("lang/language_lex_result_test.c", (
        "lang/language_lex_result.c", "analysis/syntax_span.c",
        "document/source_range.c", "syntax/role.c", "text/bounded_text.c",
    )),
    ("document/text_snapshot_test.c", (
        "document/text_snapshot.c",
    )),
    ("document/text_document_test.c", (
        "document/text_document.c", "document/text_snapshot.c",
    )),
    ("document/line_index_test.c", (
        "document/line_index.c", "document/source_position.c",
        "document/text_snapshot.c",
    )),
    ("project/project_file_test.c", (
        "project/project_file.c",
    )),
    ("project/language_project_test.c", (
        "project/language_project.c", "project/project_file.c",
        "project/build_target.c", "project/project_configuration.c",
    )),
    ("project/project_configuration_test.c", (
        "project/project_configuration.c",
    )),
    ("project/build_target_test.c", (
        "project/build_target.c",
    )),
    ("index/file_analysis_test.c", (
        "index/file_analysis.c", "semantic/symbol.c", "semantic/reference.c",
        "document/source_range.c",
    )),
    ("index/project_index_test.c", (
        "index/project_index.c", "index/file_analysis.c", "semantic/symbol.c",
        "semantic/reference.c", "document/source_range.c",
        "project/language_project.c", "project/project_file.c",
        "project/build_target.c", "project/project_configuration.c",
    )),
    ("index/dependency_graph_test.c", (
        "index/dependency_graph.c", "project/language_project.c",
        "project/project_file.c", "project/build_target.c",
        "project/project_configuration.c",
    )),
    ("lang/language_adapter_test.c", (
        "lang/language_adapter.c", "adapters/c23/c23_adapter.c",
        "adapters/c/c23_lexer.c", "adapters/c/c23_lex_step.c",
        "lang/language_lex_result.c", "analysis/syntax_span.c",
        "document/source_range.c", "syntax/role.c", "text/bounded_text.c",
    )),
    ("lang/language_registry_test.c", BASE_SOURCES),
    ("lang/language_engine_test.c", BASE_SOURCES),
    ("syntax/token_stream_test.c", (
        "syntax/token_stream.c", "lang/language_engine.c",
        "lang/language_adapter.c", "lang/language_registry.c",
        "lang/language_lex_result.c", "adapters/c23/c23_adapter.c",
        "adapters/c/c23_lexer.c", "adapters/c/c23_lex_step.c",
        "document/text_snapshot.c", "analysis/syntax_span.c",
        "document/source_range.c", "token/token.c", "syntax/role.c",
        "text/bounded_text.c",
    )),
    ("adapters/c23/c23_adapter_test.c", BASE_SOURCES),
)

with tempfile.TemporaryDirectory(prefix="c23-adapter-test-") as directory:
    for mode, flags in (("strict", ()),
                        ("asan-ubsan", ("-fsanitize=address,undefined", "-fno-omit-frame-pointer"))):
        for test_name, sources in CASES:
            binary = Path(directory) / f"{mode}-{Path(test_name).stem}"
            command = [
                "clang", "-std=c23", "-Wall", "-Wextra", "-Werror", *flags,
                f"-I{LANGUAGE}", f"-I{VEXSPOKE}", str(ROOT / "language/test" / test_name),
                *(str(LANGUAGE / source) for source in sources), "-o", str(binary),
            ]
            subprocess.run(command, check=True)
            result = subprocess.run([str(binary)], check=True, capture_output=True, text=True)
            expected = {
                "lang/language_lex_result_test.c": (0, ""),
                "document/text_snapshot_test.c": (0, ""),
                "document/text_document_test.c": (1, "TextDocument: rejected invalid update bytes"),
                "document/line_index_test.c": (2, "byte offset"),
                "project/project_file_test.c": (2, "rejected"),
                "project/language_project_test.c": (5, "duplicate file path"),
                "project/project_configuration_test.c": (1, "empty language id"),
                "project/build_target_test.c": (3, "rejected"),
                "index/file_analysis_test.c": (2, "rejected"),
                "index/project_index_test.c": (3, "ProjectIndex:"),
                "index/dependency_graph_test.c": (4, "DependencyGraph:"),
                "lang/language_adapter_test.c": (0, ""),
                "lang/language_registry_test.c": (2, "duplicate language id"),
                "lang/language_engine_test.c": (3, "language id is not registered"),
                "syntax/token_stream_test.c": (2, "TokenStream:"),
                "adapters/c23/c23_adapter_test.c": (1, "language id is not registered"),
            }[test_name]
            count, marker = expected
            if result.stderr.count("[vex]") != count or (marker and marker not in result.stderr) \
                    or (test_name == "lang/language_registry_test.c"
                        and "duplicate language id" not in result.stderr):
                raise AssertionError(f"{mode} {test_name}: unexpected diagnostics: {result.stderr!r}")
            print(f"PASS {mode} {test_name}")
