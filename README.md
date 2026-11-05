# language — R3 grammar & LSP driver provider

The source currently implements value, styling, lexical-adapter and project
configuration slices. The workspace build graph does not yet provide a runnable
Language target; a source tree or owner test is not a registered product target.

## Current State

**Role:** R3 driver — syntax backends for the ecosystem (used by `semicolon`).

**Implemented:** initial value-object scaffolding exists for source ranges,
tokens, symbols and references (`document/source_range.*`, `token/token.*`,
`semantic/symbol.*`, `semantic/reference.*`). A small unified-diff adapter
classifies patch lines for structural highlighting. The value records are
borrowed/value-only; the adapter does not lex changed source text. Neither slice
provides programming-language parsing, symbol resolution, project indexing, or
LSP services.

The first styling/analysis value layer now also exists: `BoundedText` provides
returned, truncation-flagged projections; `TextStyle` carries inherited colors,
bold/italic/dim/reverse/blink/hidden attributes and independently colored
underline/squiggle/strike decorations; `StyleKeyRegistry` joins shared style
keys and language-prefixed adapter keys through one bounded fallback; and
`SyntaxRole`, `SyntaxSpan`, and `Diagnostic` provide language-neutral outputs
for adapters and hosts. The initial C23 adapter now includes `C23Lexer`, a
borrowed-snapshot lexical span producer; `C23Highlights`, a conservative lexical
presentation classifier that keeps keyword/literal/comment roles and refines
identifier roles into type, function, variable, member and macro/constant from
local spelling and neighboring tokens, with line-start preprocessor markers and
directive names painted in the macro role and a well-formed `<...>` header name
after `#include` emitted as one string-role span; and `C23InlayHints`, a conservative
same-snapshot producer of parameter-name labels for simple direct calls. These
emit syntax roles and virtual byte anchors only; colors and presentation remain
consumer-owned. The C corpus at
`../../tests/language/adapters/c/corpus.c` records the intended cases (`*` as
multiply/dereference/declarator, macro object/function/generic claims,
`;;` claims versus empty statements, and documentation comments); the C lexer
has a strict/annotation-profile lexical subset but is not a parser or preprocessor.
`C23Highlights` is not semantic resolution: it does not expand macros, resolve
declarations, parse, or distinguish parameters from local variables beyond the
current variable role.

**New engine slice:** `lang` provides a callback adapter contract, explicit-id
registry and host-facing lexer engine. `adapters/c23` registers the existing
strict C23 lexer as `c23`; a host selects that id explicitly. The engine emits
syntax spans from borrowed snapshots and does not infer dialect from suffixes.
The document layer now owns immutable revisioned snapshots and line indexes;
the project model accepts caller-fed file inventory, language configuration
and build targets; and `index/` stages and publishes versioned per-file symbol
and reference contributions, plus a caller-fed dependency graph for transitive
invalidation queries. `syntax/TokenStream` collects adapter spans into
snapshot-bound shared tokens and rejects stale or malformed streams. These
contracts do not perform filesystem discovery or parsing yet.

Semicolon's first consumer is now a direct static C23 link through its
`src/editor/semicolon_syntax.{h,c}` bridge: copied editor rows produce live
lexical colors and bold brackets in the terminal shell, and the Language
`C23Highlights` pass distinguishes types, functions, variables/members and
macros/constants while keeping regular and documentation comments separate; an
angle-bracket header path after `#include` is a string span, while ordinary
`<`/`>` operators keep their operator role. The consumer owner is
`../../tests/semicolon/editor/run.py` (strict C23, C ASan/UBSan and scripted
PTY scopes on macOS arm64). This does not integrate the
generic registry/project engine, semantic classification, inlay hints,
diagnostics or LSP; font appearance, ligature settings and physical-terminal
approval remain unproved and host-owned.

**Still a blueprint:** project filesystem discovery, parser/AST, semantic
resolver, C23 parser, compiler service, scheduler, LSP transport and
hot-swappable grammar dylibs do not exist. The current C23 adapter is lexical
only; `TokenStream` is a validated token projection, not a parser. No runnable
repository build target is registered yet.

**Verification performed:** on Apple Silicon macOS, the language values,
revisioned document layer, project model/index, C23 lexer/hint, adapter,
registry and engine owners compile with C23 and `-Wall -Wextra -Werror`, both
normally and with ASan/UBSan. The existing C23 runner also checks constructor
arity and expected rejection diagnostics. This is local engine and data-model
evidence, not parser, filesystem-discovery, platform, or Semicolon UI
integration proof.

**Platforms proven for language services:** none. The diff classifier is a local
offline parsing helper only, not a complete language service.

## What it is
`language` owns the `Language` contract (`Lang_tokenize/parse/highlight/...`).
Each grammar ships as a hot-swappable dylib loaded through `hotcwap` R1 —
adding a language never rebuilds the IDE, it drops in a module.

## Depends on (Vertical Integration Law allowlist)
R3 may borrow either R2 public contract: Vexspoke CPU computation/behavior or
Relational Engine memory/storage, stable rows, variable bindings and native C
search over Rust-owned spans (`+ graphvex` for GPU-backed highlighting). Never
R1/R4/R5 or `api-haven` headers. This blueprint has no implemented engine
integration. Migration is staged; Vexspoke's existing memory/container ABI and
default allocator remain. R1 owns lifetimes/residency; no C/Rust atomic-layout
compatibility or automatic schema migration is assumed. GPU dispatch stays R3.

## Layout
- `document`: source ranges, immutable revisioned text snapshots/documents, and
  byte-to-line/UTF position indexing.
- `token`: shared lexical token kind/range value (`Token`).
- `semantic`: shared symbol identity/kinds (`Symbol`) and use-site
  relationship (`Reference`). Function, class, variable, macro, label and type
  aliases are currently `SymbolKind` variants, not separate classes; global
  declarations use the global scope identity chosen by a future index.
- `lang`: generic lexer adapter contract, explicit language registry and
  snapshot/context-bound engine.
- `syntax`: snapshot-bound generic `TokenStream`; parser tree/results remain
  planned.
- `project`: caller-fed project inventory, language configuration and targets;
  it performs no filesystem discovery.
- `index`: staged per-file analysis contributions, project-level symbol and
  reference queries, and caller-fed dependency-edge invalidation queries.
- `adapters/c23`: explicit C23 language-id bridge to the strict C23 lexer.
- `adapters/c`: C23 lexical/value implementation, conservative highlight roles and inlay hints.
- `adapters/<language>/`: future language frontends; `adapters/diff` contains
  the unified-diff line classifier.
- `../../tests/language`: mirrored owner tests and `run.py`; checklist evidence is
  being recorded separately for these Language tests.

## Laws that govern work here
- Constitution: the [canonical preferences.md Gist](https://gist.github.com/vex-graph/4132a6c45cb6d3797c3e8eff2e94035a); the one real, Git-ignored workspace `/PREFERENCES.md`, not a Vexspoke file or symlink.
- Commits land in THIS repo root, one cohesive unit each; never push unless asked.
- One public class per `.h`/`.c` pair, `(*ptr).field` (never `->`), no
  caller-provided destination parameters, `-Wall -Wextra -Werror`.

## Scope and Limitations

**Scope (intended):** R3 language support — per-grammar hot-swappable dylibs and
the tokenizer/AST/highlighter/LSP surface consumed by `semicolon`.

**Deliberately not covered:** filesystem discovery and IDE integration remain
consumer-owned; it never includes R1/R4/R5 or `api-haven` headers; the allocator
and IO remain R2 (Relational Engine).

**Known limits and gaps:** owner tests are strict C23 local tests, not yet an
umbrella build target. The project model and index are data contracts; there is
no filesystem scanner, parser, semantic resolver or automatic invalidation
scheduler; dependency edges must currently be supplied by an analysis host.
The C23 lexer is a lexical subset, not a parser,
preprocessor or semantic resolver; `C23Highlights` classifies identifiers from
local spelling and neighboring tokens, so it is presentation only and does not
resolve symbols or prove a role; inlay hints cover only simple same-snapshot
declarations and direct calls, omitting complex declarators, macros, function
pointers and cross-file declarations. Diff support only classifies unified Git
patch line roles; there is no changed-code lexer integration, full patch model,
claim verifier, macro expansion engine, color-theme persistence, project-wide usages, LSP transport,
full IDE integration, or non-macOS platform proof. Earlier blueprint docs still
disagree on the grammar-module ABI (`Lang_*` vs `Grammar_*` vs `LangDriver`);
these objects do not settle that dispute.
