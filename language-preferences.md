# language — Repo-Local Living Preferences

## Constitution Link

gist.md (`../../metadata/gists/preferences/gist.md`) is the compressed law list; all universal laws bind this repository. Read one on demand: `./tools/prefs law "<title>"`. Universal laws are inherited, never restated.

Tier: core (R3 Language Grammars). Owns: grammar/AST semantics, hot-swappable grammar modules. Borrows: Vexspoke computation/behavior, Relational Engine memory/storage/native C search.

## Exclusive Laws

| Law Title | Scope | Enforcement |
| **Hot-Swappable Grammar Module Law** | R3 Language Grammars | Mandatory for `language` |
| **Single-Class-Per-File Grammar Law** | R3 Language Grammars | Mandatory for `language` |

### Hot-Swappable Grammar Module Law

* Grammars are independent dynamically loadable modules exporting standard parser vtables and token iterators.
* Grammars are swappable at runtime without restarting the language server.
* Every grammar dylib exports `Grammar_init`, `Grammar_parse` and `Grammar_destroy` symbols.
* Parsers output relational AST nodes into caller-provided memory pools (stateless AST).

### Single-Class-Per-File Grammar Law

* Grammar rules, lexers, AST node builders and token definitions each occupy a dedicated `.h`/`.c` pair named for the class in lowercase.
* Each grammar file pair has exactly one public struct.

## Repo-Local Extensions

* None — no managed exceptions to universal laws.

## Readiness Cross-Reference

* Feature readiness: language readiness gist.
* Open blockers: ecosystem blockers gist.
* Proof lawbook: `tests/test-preferences.md` — `./tools/prefs test "<title>"`.
* Evidence ledger: `metadata/gists/test-checklist.md` — `python3 tools/test_checklist.py check`.
